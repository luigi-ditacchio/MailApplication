#include <stdlib.h>		//NULL
#include <string.h>		//memset
#include <sys/epoll.h>		//epoll
#include <sys/mman.h>		///mmap e costanti di mmap
#include <pthread.h>		//pthread
#include <semaphore.h>		//semafori
#include <sys/types.h>		//pthread, size_t, ecc
#include <sys/stat.h>		//fstat
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>		//O_RDWR, O_CREAT, ecc
#include <sys/socket.h>		//socket, bind, ecc
#include <netinet/in.h>		//sockaddr_in
#include <netdb.h>		//in_addr_t, in_port_t
#include <signal.h>
#include <stdio.h>

#include "workerThread.h"
#include "IOThread.h"
#include "interThreadsCommunication.h"
#include "clientState.h"
#include "signalHandling.h"
#include "timeout.h"
#include "main.h"

#define MAX_CONN 	1024		//numero massimo di connessioni attive
#define PORT 		12174		//porta su cui il server è in ascolto
#define BACKLOG 	  10		//numero massimo di connessioni da mantenere pendenti

#define ERRORE(mex) { printf(mex); exit(1);}

void createThreads();


int main() {
	
	int i;
	sigset_t sigset;
	int ds_accsock;						//Descrittore del socket dove il server accetta connessioni
	struct sockaddr_in my_addr;				//Indirizzo del server (nel dominio Internet)
	int ds_handler;				//descrittore sul quale verrà gestita la richiesta dell'host
	
	struct sockaddr host_addr;		//Indirizzo dell'host
	int host_len;				//lunghezza dell'indirizzo dell'host
	
	struct epoll_event ev;
	
	TimeoutNode *node, *to_ptr;
	
	
	
	
	initAccount();
	initThreadsCommunication();
	initLoadAdaptation();
	initMailbox();
	initTimeout();
	
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////GESTIONE SEGNALI////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	signalHandling();
	
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGUSR1);
	sigaddset(&sigset, SIGUSR2);
	
	if (pthread_sigmask( SIG_BLOCK, &sigset, NULL) != 0)
		ERRORE("Errore nella gestione della maschera dei segnali del main thread\n")
	
	
	

	////////////////////////////////////////////////////////////////////
	///////////////INIZIALIZZAZIONE DELLA STRUTTURA EPOLL///////////////
	////////////////////////////////////////////////////////////////////
	
	ds_epoll = epoll_create(MAX_CONN);
	if ( ds_epoll == -1)
		ERRORE("Errore nella creazione della struttura epoll")
	
	
	////////////////////////////////////////////////////////////////////
	//////////////////////CREAZIONE DEI THREADS/////////////////////////
	////////////////////////////////////////////////////////////////////
	
	createThreads();
	
	////////////////////////////////////////////////////////////////////
	///////////////////INIZIALIZZAZIONE DEL SOCKET//////////////////////
	////////////////////////////////////////////////////////////////////
	
	
	
	ds_accsock = socket(AF_INET, SOCK_STREAM, 0);
	if (ds_accsock == -1) 
		ERRORE("Errore nella creazione del socket")
		
		
	memset((void *)&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	
	if (bind(ds_accsock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in)) ==  -1) 
		ERRORE("Errore nel binding dell'indirizzo")
	
	if (listen(ds_accsock, BACKLOG) == -1) 
		ERRORE("Errore nell'impostazione del numero massimo di connessioni pendenti")
		
	
	
		
	////////////////////////////////////////////////////////////////////
	///////////////GESTIONE DELLE RICHIESTE ENTRANTI////////////////////
	////////////////////////////////////////////////////////////////////
		
	
	
	
	alarm(TIMEOUT);

	while (1) {
		
		while (  (ds_handler = accept(ds_accsock, &host_addr, &host_len)) == -1) {
			if (errno != EINTR && errno != ECONNABORTED ) {
				printf("Errore sconosciuto nella accept\n");
			}
		}
		
		//Incremento il contatore di clients connessi
		pthread_mutex_lock(&clientsconn_mutex);
		clients_connected++;
		pthread_mutex_unlock(&clientsconn_mutex);
		//fcntl(ds_handler, F_SETFL, O_NONBLOCK);
		#ifdef DEBUG
			printf("Nuova connessione su socket %d\n", ds_handler);
		#endif
		
		ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET | EPOLLONESHOT;
		ev.data.ptr = (ClientState *)malloc(sizeof(ClientState));
		((ClientState *)ev.data.ptr)->ds_sock = ds_handler;
		((ClientState *)ev.data.ptr)->authenticated = 0;
		((ClientState *)ev.data.ptr)->alive = 1;
		((ClientState *)ev.data.ptr)->wr_block = NULL;

		addNodeTimeout((ClientState *)ev.data.ptr);
		

		epoll_ctl(ds_epoll, EPOLL_CTL_ADD, ds_handler, &ev);

		
		
	}
		
	

	 
	
}

void createThreads() {

	pthread_t thread_id;
	int i;
	
	for(i=0; i<IOTHREADS_MIN; i++) {
		pthread_create(&thread_id, NULL, IOThread, NULL);
	}
	
	pthread_mutex_lock(&ionum_mutex);
	iothreads_num++;
	pthread_mutex_unlock(&ionum_mutex);
	
	for(i=0; i<WORKER_THREADS_MIN; i++) {
		pthread_create(&thread_id, NULL, workerThread, NULL);
	}
	
	pthread_mutex_lock(&worknum_mutex);
	workerthreads_num++;
	pthread_mutex_unlock(&worknum_mutex);
}
	
	

