#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>

#include "workerThread.h"
#include "interThreadsCommunication.h"
#include "protocol.h"
#include "mailbox.h"
#include "account.h"

unsigned char checkNameAndPsw(char * data, int size, char * * psw);
char checkMail(ClientState * state, char * data, int size, char * * dest);

void * workerThread(void * arg) {

	
	FileLock * slot;		//rappresenta lo slot di un SCL dove il thread mantiene il lock sul file
	char * path;			//è dove viene memorizzato l'indirizzo del file da aprire
	char * mailbox;			//verranno memorizzati i messaggi della mailbox da spedire all'utente
	char * help_ptr;		//puntatore ausiliario: può puntare alla psw nel login/registrazione o al destinatario nell'invio di una mail
	struct stat * filestat;		//è dove vengono memorizzate le informazioni sul file aperto (tra cui la taglia)
	int partial_read, total_read, partial_written, total_written;		//utilizzati nelle operazioni di read e write
	int checkmail_return;
	char * dest;
	//Variabili utilizzate per input work
	ClientState * state;
	void * data;
	int size;
	
	sigset_t sigset;
	
	
	//Blocco il SIGUSR1 che è destinato agli io threads
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGUSR1);
	
	if (pthread_sigmask( SIG_BLOCK, &sigset, NULL) != 0) {
		printf("Errore nella gestione della maschera dei segnali dell'iothread\n");
		exit(1);
	}
	
	//Creo una maschera con il solo SIGUSR2, per bloccarlo e sbloccarlo velocemente
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGUSR2);
	

	while (1) {
	
		if (pthread_sigmask( SIG_UNBLOCK, &sigset, NULL) != 0) {
			printf("Errore nella gestione della maschera dei segnali dell'iothread\n");
			exit(1);
		}
		
		if (pthread_sigmask( SIG_BLOCK, &sigset, NULL) != 0) {
			printf("Errore nella gestione della maschera dei segnali dell'iothread\n");
			exit(1);
		}
	
		#ifdef DEBUG
			printf("Worker thread %u: Controllo la presenza di richieste da processare\n", (unsigned int)pthread_self());
		#endif


		getInput(&state, &data, &size);
		
		#ifdef DEBUG
			printf("Thread %u: Terminato get input, indirizzo pacchetto: %p\n", (unsigned int)pthread_self(), data);
		#endif
		
		switch( *(ReqCode *)data ) {
		
		
		case LOGIN_REQ:
		
			#ifdef DEBUG
				printf("Socket %d: Richiesta di login\n", state -> ds_sock);
			#endif	
		
			if ( !checkNameAndPsw((char *)data, size, &help_ptr) ) {
				addOutputCode(state, INVALID_FORMAT);
				break;
			}
			
			#ifdef DEBUG
				printf("Socket %d: Login di nome: %s e password: %s\n", state -> ds_sock, (char *)data + sizeof(ReqCode), help_ptr);
			#endif
				
			if ( login( (char *)data + sizeof(ReqCode), help_ptr) == 1  ) {

				#ifdef DEBUG
					printf("Socket %d: Client autenticato\n", state -> ds_sock);
				#endif	
		
				//Aggiornamento dello stato
				state -> name = malloc(strlen( data + sizeof(ReqCode)) + 1 );
				strcpy( state -> name, data + sizeof(ReqCode) );
				state -> authenticated = 1;

		
				addOutputCode( state, LOGIN_OK);
				
		
			}

			else {
				#ifdef DEBUG
					printf("Socket %d: Client non autenticato\n", state -> ds_sock);
				#endif	
		
				addOutputCode(state, LOGIN_ERROR);
			}
			
			break;
			
			
			
		case REG_REQ:
		
			#ifdef DEBUG
				printf("Socket %d: Richiesta di registrazione \n", state -> ds_sock );
			#endif
			
			if ( !checkNameAndPsw((char *)data, size, &help_ptr) ) {
				addOutputCode(state, INVALID_FORMAT);
				break;
			}
			
			
			#ifdef DEBUG
				printf("Socket %d: Registrazione di nome: %s e password: %s\n", state -> ds_sock, (char *)data + sizeof(ReqCode), help_ptr);
			#endif


			switch (  registration( data + sizeof(ReqCode), help_ptr ) ) {

			case 1:
				#ifdef DEBUG
					printf("Socket %d: Client registrato\n", state -> ds_sock);
				#endif
				
				//Aggiornamento dello stato
				state -> name = malloc(strlen( data + sizeof(ReqCode)) + 1 );
				strcpy( state -> name, data + sizeof(ReqCode) );
				state -> authenticated = 1;
				
				addOutputCode(state, REG_OK);
				break;
			
			
			case 0:
				#ifdef DEBUG
					printf("Socket %d: Client non registrato per nome già esistente\n", state -> ds_sock);
				#endif
				addOutputCode( state, REG_ERROR);
				break;


			case -1:
				#ifdef DEBUG
					printf("Socket %d: Client non registrato per un errore interno\n", state -> ds_sock);
				#endif
				addOutputCode( state, INTERNAL_ERROR);
				break;

			
			}
			
			break;
			
		
		
		
		case RECV:
		
			#ifdef DEBUG
				printf("Socket %d: Richiesta di ricezione messaggi\n", state -> ds_sock );
			#endif
			
			path = (char *)malloc(strlen( state -> name) + 20);
			strcpy(path, "./data/mailbox/");
			strcat(path, state -> name );
			strcat(path, ".rtf");
	
	
			slot = lockFile( state -> name );

			if ( (slot->ds_file = open(path, O_RDWR | O_CREAT, 0666)) == -1) {
				printf("Errore nell'apertura di un file utente\n");
				free(path);
				addOutputCode( state, INTERNAL_ERROR);
				break;
			}
			free(path);
			filestat = (struct stat *)malloc( sizeof(struct stat) );
			fstat(slot->ds_file, filestat);
			mailbox = (char *)malloc(filestat->st_size + 4);
			
			total_read = 0;
			do {
				partial_read = read(slot->ds_file, mailbox + total_read, filestat->st_size - total_read);
				if (partial_read >= 0) {
					total_read += partial_read;
				}
				else {
					if (errno != EINTR) {
						printf("Errore sconosciuto nella lettura di un file utente\n");
						free(filestat);
						free(mailbox);
						addOutputCode( state, INTERNAL_ERROR );
						break;
					}
				}
			} while (filestat->st_size - total_read > 0);
			
			
			if (partial_read == -1) break;
			
			#ifdef DEBUG
				printf("Socket %d: Lettura mailbox eseguita con successo\n", state -> ds_sock );
			#endif
			
			while ( close(slot->ds_file) == -1) {
				if (errno != EINTR) {
					printf("Errore nella chiusura del file %d\n",slot->ds_file);
					break;
				}
			}
			unlockFile(slot);
			
			strncpy( mailbox + filestat->st_size, "\r\n\r\n", 4);
			
			
			addOutput( state, mailbox, filestat->st_size + 4);
			
			free(filestat);
			break;
		
		
		case RMV:
		
			#ifdef DEBUG
				printf("Socket %d: Richiesta di rimozione messaggi\n", state -> ds_sock );
			#endif
			
			path = (char *)malloc(strlen(state -> name) + 20);
			strcpy(path, "./data/mailbox/");
			strcat(path, state -> name );
			strcat(path, ".rtf");
	
	
			slot = lockFile( state->name);
			
			
			if ( (slot->ds_file = open(path,  O_RDWR | O_CREAT | O_TRUNC, 0666)) == -1) {
				printf("Errore nell'apertura di un file utente\n");
				free(path);
				addOutputCode( state, INTERNAL_ERROR);
				break;
			}
			
			#ifdef DEBUG
				printf("Socket %d: Rimozione messaggi eseguita con successo\n", state -> ds_sock );
			#endif
				
			free(path);
	
			
			while ( close( slot->ds_file) == -1) {
				if (errno != EINTR) {
					printf("Errore nella chiusura del file %d\n",slot->ds_file);
					break;
				}
			}
			
			unlockFile(slot);
		
			addOutputCode( state, OP_OK );
		
			break;

			
			
			
		case SEND:
			
			#ifdef DEBUG
				printf("Socket %d: Richiesta di invio di un messaggio\n", state -> ds_sock );
			#endif
			
			checkmail_return = checkMail(state, data, size, &dest);
			
			switch (checkmail_return) {
			
			case 1:
				break;
			
			case 0:
				addOutputCode(state, INVALID_FORMAT);
				break;
			
			case -1:
				addOutputCode(state, INVALID_SENDER);
				break;
				
			case -2:
				addOutputCode(state, INVALID_RECEIVER);
				break;
			
			}
			
			if (checkmail_return != 1) break;
			

			//Memorizzazione della mail nella mailbox del destinatario
			path = (char *)malloc(strlen(dest) + 20);
			strcpy(path, "./data/mailbox/");
			strcat(path, dest);
			strcat(path, ".rtf");
			
			
			slot = lockFile(dest);
			if (slot == NULL) {
				printf("Errore sconosciuto nel lock di un file\n");
				raise(SIGTERM);
			}

			if (  (slot->ds_file = open(path, O_RDWR | O_CREAT | O_APPEND, 0666)) == -1  ) {
				printf("Errore sconosciuto nell'apertura di un file utente\n");
				unlockFile(slot);
				free(path);
				addOutputCode( state, INTERNAL_ERROR);
				break;
			}
			
			
			free(path);
			
			
			total_written = 0;
			do {
				partial_written = write( slot->ds_file, data + sizeof(ReqCode) + total_written, size - sizeof(ReqCode) - total_written);
				if (partial_written >= 0) {
					total_written += partial_written;
				}
				else {
					if (errno != EINTR) {
						printf("Errore sconosciuto nella scrittura su un file utente\n");
						while ( close( slot->ds_file) == -1) {
							if (errno != EINTR) {
								printf("Errore nella chiusura del file %d\n", slot->ds_file);
								break;
							}
						}
						unlockFile(slot);
						addOutputCode( state, INTERNAL_ERROR);
						break;
					}
				}
			} while (size - sizeof(ReqCode) - total_written > 0);
			
			if (partial_written == -1) break; 
			
			fsync(slot->ds_file);
			
			#ifdef DEBUG
				printf("Socket %d: Scrittura messaggio nella mailbox eseguita con successo\n", state -> ds_sock);
			#endif


			while ( close( slot->ds_file) == -1) {
				if (errno != EINTR) {
					printf("Errore nella chiusura del file %d\n", slot->ds_file);
					break;
				}
			}
			unlockFile(slot);
			
			addOutputCode( state, OP_OK);
			break;
		
		
		}
		
		free(data);
		
		
	}
	
}






unsigned char checkNameAndPsw(char * data, int size, char * * psw) {
	
	int i;


	//Controllo del nome
	for (i = sizeof(ReqCode);  i < size && i < MAX_NAME + 5; i++) {
		if (data[i] == '\0') {
			i++;
			*psw = data + i;
			//Controllo della password	
			for (;  i < size && i < MAX_NAME + MAX_PSW + 6; i++) {
				if (data[i] == '\0') {
					return 1;
				}
			}
			break;
		}
	}
	
	return 0;
	
	
}
			


char checkMail(ClientState * state, char * data, int size, char * * dest) {

	//Codici di errore:
	//1: pacchetto corretto
	//0: INVALID_FORMAT
	//-1: INVALID_SENDER
	//-2: INVALID_RECEIVER

	char * ptr = data + sizeof(ReqCode);
	int i;
	
	data += size;
	
	//Controllo campo FROM
	if (  !(ptr+5 < data && strncmp(ptr, "From:", 5) == 0)  ) {
		return 0;
	}
	ptr += 5;
	
	for (i=0; ptr+i < data && i<MAX_NAME+1; i++) {
		if ( ptr[i]=='\0') {
			break;
		}
	}
	if ( !(ptr+i < data && i<MAX_NAME+1 )  ) {
		return 0;
	}
	
	if ( !(strcmp(ptr, state -> name ) == 0)  ) {
		return -1;
	}
	
	ptr += i + 1;
	
			
	#ifdef DEBUG
		printf("Socket %d: Campo FROM corretto\n",  state -> ds_sock);
	#endif	
	

	//Controllo campo TO
	if ( !(ptr + 3 < data && strncmp(ptr, "To:", 3) == 0) ) {
		return 0;
	}
	ptr += 3;
	
	for (i=0; ptr+i < data && i<MAX_NAME+1; i++) {
		if ( ptr[i]=='\0') {
			break;
		}
	}
	if ( !(ptr+i < data && i<MAX_NAME+1 )  ) {
		return 0;
	}
	
	
	if ( !(login(ptr, NULL) == 0)  ) {
		return -2;
	}
	
	
	*dest = ptr;
	ptr += i + 1;
	
	#ifdef DEBUG
		printf("Socket %d: Campo TO corretto\n", state -> ds_sock);
	#endif

	//Controllo campo SUBJECT
	if ( !(ptr+8 < data && strncmp(ptr, "Subject:", 8) == 0) ) {
		return 0;
	}
	ptr += 8;
	
	for (i=0; ptr+i < data && i<MAX_SUBJECT+1; i++) {
		if ( ptr[i]=='\0') {
			break;
		}
	}
	if ( !(ptr+i < data && i<MAX_SUBJECT+1 ) ) {
		return 0;
	}
	ptr += i + 1;

	#ifdef DEBUG
		printf("Socket %d: Campo SUBJECT corretto\n", state -> ds_sock);
	#endif					
	
	//Controllo campo BODY
	for (i=0; ptr+i < data && i<MAX_BODY+1; i++) {
		if ( ptr[i]=='\0') {
			break;
		}
	}
	
	if ( !(ptr+i < data && i<MAX_BODY+1 ) ) {
		return 0;
	}

	
	#ifdef DEBUG
		printf("Socket %d: Campo BODY corretto\n", state -> ds_sock);
	#endif
	
	return 1;
}



