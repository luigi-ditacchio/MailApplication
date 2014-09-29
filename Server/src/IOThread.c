#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "IOThread.h"
#include "interThreadsCommunication.h"
#include "timeout.h"
#include "clientState.h"
#include "protocol.h"
#include "main.h"


#define CLIENTSTATE   ((ClientState *)ev.data.ptr)
#define EPOLL_TIMEOUT  1000


void tryToSend( ClientState *state, void * data, int size );
void tryToSendCode(ClientState * state, ReplyCode code);



void * IOThread(void * arg) {

	int epoll_return;				//valore di ritorno della funzione epoll_wait
	int received;					//numero di bytes letti
	void * packet;					//dove viene memorizzato il pacchetto ricevuto
	ReqCode operation;				//operazione richiesta dal client
	
	struct epoll_event ev;				//evento epoll
	
	sigset_t sigset;				//maschera di segnali per IOThread
	
	int partial_sent, total_sent;
	
	ClientState * state;
	void * data;
	int size;
	
	//Blocco il SIGUSR2 che è destinato ai worker threads
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGUSR2);
	
	if (pthread_sigmask( SIG_BLOCK, &sigset, NULL) != 0) {
		printf("Errore nella gestione della maschera dei segnali dell'iothread\n");
		exit(1);
	}
	
	//Creo una maschera con il solo SIGUSR1, per bloccarlo e sbloccarlo velocemente
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGUSR1);
	
	
	while (1) {
		
		
		if (pthread_sigmask( SIG_UNBLOCK, &sigset, NULL) != 0) {
			printf("Errore nella gestione della maschera dei segnali dell'iothread\n");
			exit(1);
		}
		
		if (pthread_sigmask( SIG_BLOCK, &sigset, NULL) != 0) {
			printf("Errore nella gestione della maschera dei segnali dell'iothread\n");
			exit(1);
		}
		
		
		while ( getOutput(&state, &data, &size) ) {
			tryToSend( state, data, size);
		}
					
		
		
		#ifdef DEBUG
			printf("Thread %u: In attesa di un evento\n", (unsigned int)pthread_self());
		#endif
		
		

		//ATTENDO EVENTO//
		while (  (epoll_return = epoll_wait(ds_epoll, &ev, 1, EPOLL_TIMEOUT)) == -1) {
			if (errno != EINTR) {
				printf("Errore sconosciuto su epoll_wait\n");
				raise(SIGTERM);
			}
		}
		
		if (epoll_return == 0) continue;
		
		pthread_sigmask(SIG_BLOCK, &sigset, NULL);

		#ifdef DEBUG
			printf("Thread %u: C'è stato un evento\n", (unsigned int)pthread_self());
		#endif
		
		
		
		////DISCONNESSIONE O ERRORE////
		if (ev.events & EPOLLRDHUP || ev.events & EPOLLERR || ev.events & EPOLLHUP) {
		
			#ifdef DEBUG
				printf("Socket %d: Il client si è disconnesso\n", CLIENTSTATE -> ds_sock);
			#endif

			closeAndFree(CLIENTSTATE, 1);
			
			continue;
		}
		
		
		
		
		////EVENTO DI SCRITTURA////
		else if (ev.events & EPOLLOUT) {

			#ifdef DEBUG
				printf("Socket %d: Evento di scrittura\n", CLIENTSTATE -> ds_sock);
			#endif
			
			do {
				partial_sent =  send(  CLIENTSTATE -> ds_sock,  ((CLIENTSTATE->wr_block) -> data) + total_sent,  ((CLIENTSTATE->wr_block) -> size) - total_sent, MSG_DONTWAIT );
				if (partial_sent > 0) {
					total_sent += partial_sent;
				}
			} while ( ((CLIENTSTATE->wr_block) -> size) - total_sent > 0    &&    (partial_sent != -1 || errno == EINTR));
			
			if (partial_sent == -1) {
				if (errno == EAGAIN) {
					#ifdef DEBUG
						printf("Socket %d: Buffer pieno, scrittura rimandata\n", CLIENTSTATE -> ds_sock);
					#endif
					ev.events = EPOLLOUT | EPOLLRDHUP | EPOLLET | EPOLLONESHOT;
					epoll_ctl(ds_epoll, EPOLL_CTL_MOD, CLIENTSTATE -> ds_sock, &ev);
				}
				else {
					printf("Errore sul socket %d, chiusura in corso\n", CLIENTSTATE -> ds_sock);
					
					closeAndFree(CLIENTSTATE, 1);
				}
				continue;
			}
			
			
			free(CLIENTSTATE -> wr_block);
			CLIENTSTATE -> wr_block = NULL;
			ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET | EPOLLONESHOT;
			epoll_ctl(ds_epoll, EPOLL_CTL_MOD, CLIENTSTATE -> ds_sock, &ev);
			#ifdef DEBUG
				printf("Socket %d: Deallocazione memoria completata\n", CLIENTSTATE -> ds_sock);
			#endif
				
				
		}
	
		
		////EVENTO LETTURA////
		else if (ev.events & EPOLLIN) {

			#ifdef DEBUG
				printf("Socket %d: Dati da leggere\n", CLIENTSTATE -> ds_sock);
			#endif

			CLIENTSTATE -> alive = 1;
			
			
			if (  !(CLIENTSTATE->authenticated) ) {
				
				#ifdef DEBUG
					printf("Socket %d: Client non autenticato\n", CLIENTSTATE -> ds_sock);
				#endif
		   
				packet = malloc(MAX_AUTH);
			
				if (   (received = recv(  CLIENTSTATE -> ds_sock, packet, MAX_AUTH, MSG_DONTWAIT  )  ) == -1  ) {
					printf("Errore sul socket %d, chiusura in corso..\n", CLIENTSTATE -> ds_sock);	
					
					closeAndFree(CLIENTSTATE, 1);
					
					free(packet);
					continue;
				}
				
				#ifdef DEBUG
					printf("Socket %d: Letto pacchetto di autorizzazione\n", CLIENTSTATE -> ds_sock);
				#endif
		   
			
				while ( recv( CLIENTSTATE -> ds_sock, NULL, 4096, MSG_DONTWAIT) != -1) {}
					if (errno != EAGAIN) {
						printf("Errore sul socket %d, chiusura in corso..\n", CLIENTSTATE -> ds_sock);
						
						closeAndFree(CLIENTSTATE, 1);
						
						free(packet);
						continue;
					}
				#ifdef DEBUG
					printf("Socket %d: Svuotamento del buffer lettura ultimato\n", CLIENTSTATE -> ds_sock);
				#endif	
			
				if (received < MIN_AUTH || ( *(ReqCode *)packet != LOGIN_REQ && *(ReqCode *)packet != REG_REQ ) ) {
					tryToSendCode( CLIENTSTATE, INVALID_FORMAT );
					free(packet);
					continue;
				}
				
				#ifdef DEBUG
					printf("Socket %d: Codice di autorizzazione presente e corretto\n", CLIENTSTATE -> ds_sock);
				#endif
				
				
				/***********************************************************
				 *            AGGIUNTA LAVORO PER WORKER THREAD            *
				 ***********************************************************/
				 #ifdef DEBUG
					printf("Socket %d: Aggiunta lavoro per worker thread, indirizzo pacchetto: %p\n", CLIENTSTATE -> ds_sock, packet);
				#endif
				
				addInput(CLIENTSTATE, packet, received);
				
				
			}
			
			////IL CLIENT E' STATO GIA' AUTENTICATO////
			else {
				#ifdef DEBUG
					printf("Socket %d: Il client è già autenticato\n", CLIENTSTATE -> ds_sock);
				#endif
			
				if (   (received = recv(  CLIENTSTATE -> ds_sock, (void *)&operation, sizeof(ReqCode), MSG_DONTWAIT  )  ) == -1  ) {
					printf("Errore sul socket %d, chiusura in corso..\n", CLIENTSTATE -> ds_sock);	
					
					closeAndFree(CLIENTSTATE, 1);
					
					continue;
				}

				#ifdef DEBUG
					printf("Socket %d: Codice dell'operazione letto con successo\n", CLIENTSTATE -> ds_sock);
				#endif
			
				
				if (received < sizeof(ReqCode) || ( operation != SEND && operation != RECV && operation != RMV ) ) {
					tryToSendCode( CLIENTSTATE, INVALID_FORMAT );
					continue;
				}

				#ifdef DEBUG
					printf("Socket %d: Codice dell'operazione valido\n", CLIENTSTATE -> ds_sock);
				#endif
		
			  	switch( operation ) {
			  	
			  	case RECV:
			  	case RMV:
			  		
			  		#ifdef DEBUG
						printf("Socket %d: Operazione di ricezione messaggi\n", CLIENTSTATE -> ds_sock);
					#endif

					while ( recv( CLIENTSTATE -> ds_sock, NULL, 4096, MSG_DONTWAIT) != -1) {}
					if (errno != EAGAIN) {
						printf("Errore sul socket %d, chiusura in corso..\n", CLIENTSTATE -> ds_sock);
						
						closeAndFree(CLIENTSTATE, 1);
						
						continue;
					}

					#ifdef DEBUG
						printf("Socket %d: Svuotamento del buffer lettura ultimato\n", CLIENTSTATE -> ds_sock);
					#endif	
			
				  	/***********************************************************
					 *            AGGIUNTA LAVORO PER WORKER THREAD            *
					 ***********************************************************/
					addInputCode(CLIENTSTATE, operation); 
					
					break;
					
				case SEND:
				
					packet = (char *)malloc(sizeof(ReqCode) + MAX_MAIL);
					*(ReqCode *)packet = SEND;
					if (   (received = recv( CLIENTSTATE -> ds_sock, packet + sizeof(ReqCode), MAX_MAIL, MSG_DONTWAIT)) == -1  ) {
						if (errno == EAGAIN) {
							tryToSendCode( CLIENTSTATE, INVALID_FORMAT);
						
						}
						else {
							printf("Errore sconosciuto sul socket %d, chiusura in corso..\n", ((ClientState *)ev.data.ptr) -> ds_sock);
						
							closeAndFree(CLIENTSTATE, 1);
						
						}
						free(packet);
						break;
					}
					#ifdef DEBUG
						printf("Socket %d: Mail letta con successo, taglia della mail: %d bytes\n", CLIENTSTATE -> ds_sock, received);
					#endif	
					while ( recv( CLIENTSTATE -> ds_sock, NULL, 4096, MSG_DONTWAIT) != -1) {}
					if (errno != EAGAIN) {
						printf("Errore sconosciuto sul socket %d, chiusura in corso..\n", ((ClientState *)ev.data.ptr) -> ds_sock);
						
						closeAndFree(CLIENTSTATE, 1);
						
						free(packet);
						break;
					}

					#ifdef DEBUG
						printf("Socket %d: Svuotamento del buffer lettura ultimato\n", CLIENTSTATE -> ds_sock);
					#endif	
					
					
					/***********************************************************
					 *            AGGIUNTA LAVORO PER WORKER THREAD            *
					 ***********************************************************/
					addInput(CLIENTSTATE, packet, received + sizeof(ReqCode));
					
					break;
					
				}
				
			}
			
		}
		
	}
			


}



void tryToSend( ClientState *state, void * data, int size ) {

	struct epoll_event ev;
	
	#ifdef DEBUG
		printf("Socket %d: Provo ad inviare mail\n", state -> ds_sock);
	#endif
	if (   send(state->ds_sock, data, size, MSG_DONTWAIT)  ==  -1  ) {
		if (errno == EAGAIN) {
			#ifdef DEBUG
				printf("Socket %d: Buffer scrittura pieno, aggiungo ad epoll\n", state -> ds_sock);
			#endif
			state->wr_block = (WriteBlock *)malloc(sizeof(WriteBlock));
			(state->wr_block)->data = data;
			(state->wr_block)->size = size;
			
			ev.events = EPOLLOUT | EPOLLRDHUP | EPOLLET | EPOLLONESHOT;
			ev.data.ptr = (void *)state;
			epoll_ctl(ds_epoll, EPOLL_CTL_MOD, state->ds_sock, &ev);
			return;
		}
		else {
			printf("Errore sul socket %d, chiusura in corso\n", state -> ds_sock);
			
			closeAndFree(CLIENTSTATE, 1);
			
			return;
		}
	}
	#ifdef DEBUG
		printf("Socket %d: Scrittura mail conclusasi con successo\n", state -> ds_sock);
	#endif
	ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET | EPOLLONESHOT;
	ev.data.ptr = (void *)state;
	epoll_ctl(ds_epoll, EPOLL_CTL_MOD, state->ds_sock, &ev);
	
}

void tryToSendCode(ClientState * state, ReplyCode code) {

	ReplyCode * data = (ReplyCode *)malloc(sizeof(ReplyCode));
	*data = code;
	
	tryToSend(state, data, sizeof(ReplyCode));
	
}


void closeAndFree(ClientState * state, unsigned char timeout) {
	
	#ifdef DEBUG
		printf("Socket %d: Chiusura e liberazione memoria in corso\n", state -> ds_sock);
	#endif
		
	//Rimuovo da epoll
	epoll_ctl(ds_epoll, EPOLL_CTL_DEL, state -> ds_sock, NULL);
	
	while ( close(state -> ds_sock) == -1 && errno==EINTR) {}
	
	//Decremento il contatore di clients connessi
	pthread_mutex_lock(&clientsconn_mutex);
	clients_connected--;
	pthread_mutex_unlock(&clientsconn_mutex);
	
	if (state -> authenticated) {
		free( state-> name );
	}
	if (state -> wr_block != NULL) {
		free( (state -> wr_block) -> data);
		free( state -> wr_block );
	}
	
	if (timeout) removeNodeTimeout(state, 1);
	
	free(state);
	
	#ifdef DEBUG
		printf("Thread %u: CloseAndFree completata\n", (unsigned int)pthread_self());
	#endif
	
}
