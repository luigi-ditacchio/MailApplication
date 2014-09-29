#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "signalHandling.h"
#include "timeout.h"
#include "protocol.h"
#include "workerThread.h"
#include "interThreadsCommunication.h"
#include "IOThread.h"
#include "loadAdaptation.h"
#include "account.h"
#include "mailbox.h"

#define STATE (ptr->state)
#define NEXTSTATE ((ptr->next)->state)

int alarm_activations;

void alarmHandling(int signo);
void safeClose(int signo, siginfo_t * info, void * context);
void fastClose(int signo);
void usr1Handling(int signo);
void usr2Handling(int signo);


void signalHandling() {

	struct sigaction sa;
	sigset_t set;
	
	alarm_activations = 0;
	
	sigfillset(&set);
	sa.sa_mask = set;
	//SIGFPE, SIGILL, SIGSEGV, SIGBUS,
	//SIGABRT, SIGTRAP, SIGSYS
	
	sa.sa_sigaction = safeClose;
	sa.sa_flags = SA_SIGINFO;
	
	if ( sigaction(SIGFPE, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	if ( sigaction(SIGILL, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	if ( sigaction(SIGSEGV, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	if ( sigaction(SIGBUS, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	if ( sigaction(SIGABRT, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	if ( sigaction(SIGTRAP, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	if ( sigaction(SIGSYS, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	
	
	//SIGTERM, SIGINT, SIGQUIT, SIGHUP
	sa.sa_handler = fastClose;
	sa.sa_flags = 0;
	
	if ( sigaction(SIGTERM, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	if ( sigaction(SIGINT, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	if ( sigaction(SIGQUIT, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	if ( sigaction(SIGHUP, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	
	
	//SIGPIPE//
	sa.sa_handler = SIG_IGN;
	if ( sigaction(SIGPIPE, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	
	//SIGALRM//
	sa.sa_handler = alarmHandling;
	sa.sa_flags = SA_RESTART;

	if ( sigaction(SIGALRM, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	
	//SIGUSR1//
	sa.sa_handler = usr1Handling;
	sa.sa_flags = 0;
	if ( sigaction(SIGUSR1, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	
	//SIGUSR2//
	sa.sa_handler = usr2Handling;
	sa.sa_flags = 0;
	if ( sigaction(SIGUSR2, &sa, NULL) == -1) {
		printf("Errore nella sigaction\n");
		exit(1);
	}
	
}

void alarmHandling(int signo) {
	
	
	loadAdaptation();
	
	alarm_activations++;
	
	if (alarm_activations >= 60) {
		alarm_activations = 0;
		timeout();
	}


	alarm(TIMEOUT);
}


void usr1Handling(int signo) {
	
	#ifdef DEBUG
		printf("Terminazione thread %u\n", (unsigned int)pthread_self() );
	#endif
	pthread_mutex_lock(&reg_mutex);
	sem_wait(&reg_semaphore);
	
	iothreads_num--;
	
	pthread_mutex_unlock(&reg_mutex);
	
	pthread_exit(NULL);
	
}

void usr2Handling(int signo) {

	#ifdef DEBUG
		printf("Terminazione thread %u\n", (unsigned int)pthread_self() );
	#endif

	removeSlot();
	
	pthread_exit(NULL);

}	
	

//Routine di gestione dei segnali: SIGFPE (errore aritmetico), SIGILL (illegal instruction),
//				    SIGSEGV (segment violation), SIGBUS (bus error),
//				    SIGABRT (abort), SIGTRAP e SIGSYS (system call)

void safeClose(int signo, siginfo_t *info, void *context) {

	int i;
	struct sigaction sa;
	FileLock * ptr;
	
	fsync(ds_reg);
	
	ptr = first_filelock;
	while (ptr != NULL) {
		if (ptr->ds_file != -1) {
			fsync(ptr->ds_file);
		}
		ptr = ptr->next;
	}
	
	
	pthread_mutex_destroy(&reg_mutex);
	pthread_mutex_destroy(&filelock_mutex);
	pthread_mutex_destroy(&timeout_mutex);
	pthread_mutex_destroy(&onlyone_mutex);
	pthread_mutex_destroy(&input_mutex);
	pthread_mutex_destroy(&output_mutex);
	
	
	sem_destroy(&reg_semaphore);
	
	ptr = first_filelock;
	while (ptr != NULL) {
		pthread_mutex_destroy( &(ptr->mutex));
		ptr = ptr->next;
	}
	
	pthread_cond_destroy(&input_ready);
	
	
	
	
	printf("Catturato segnale numero %d\n", signo);
	if (signo==SIGFPE || signo==SIGILL || signo == SIGSEGV || signo == SIGBUS) {
		printf("Errore avvenuto all'indirizzo %p\n", info->si_addr);
	}
	
	sa.sa_handler = SIG_DFL;
	sigaction(signo, &sa, NULL);
	
	raise(signo);
	
}


//Routine di gestione dei segnali: SIGTERM (terminate), SIQINT (interrupt),
//				    SIGQUIT, SIGHUP (hang-up)

void fastClose(int signo) {

	int i;
	FileLock * ptr;
	
	fsync(ds_reg);
	
	ptr = first_filelock;
	while (ptr != NULL) {
		if (ptr->ds_file != -1) {
			fsync(ptr->ds_file);
		}
		ptr = ptr->next;
	}
	
	pthread_mutex_destroy(&reg_mutex);
	pthread_mutex_destroy(&filelock_mutex);
	pthread_mutex_destroy(&timeout_mutex);
	pthread_mutex_destroy(&onlyone_mutex);
	pthread_mutex_destroy(&input_mutex);
	pthread_mutex_destroy(&output_mutex);
	
	
	sem_destroy(&reg_semaphore);
	
	ptr = first_filelock;
	while (ptr != NULL) {
		pthread_mutex_destroy( &(ptr->mutex));
		ptr = ptr->next;
	}
	
	pthread_cond_destroy(&input_ready);
	
	printf("Catturato segnale numero %d\n", signo);
	
	exit(0);
	
}



