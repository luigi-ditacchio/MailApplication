#include <signal.h>
#include <pthread.h>
#include <stdio.h>

#include "loadAdaptation.h"
#include "IOThread.h"
#include "workerThread.h"
#include "main.h"
#include "account.h"
#include "mailbox.h"
#include "interThreadsCommunication.h"


void initLoadAdaptation() {
	
	pthread_mutex_init(&ionum_mutex, NULL);
	pthread_mutex_init(&worknum_mutex, NULL);
	pthread_mutex_init(&clientsconn_mutex, NULL);
	pthread_mutex_init(&inputworks_mutex, NULL);
	
	clients_connected = 0;
	input_works = 0;
	iothreads_num = 0;
	workerthreads_num = 0;

}


void loadAdaptation() {

	pthread_t thread_id;

	pthread_mutex_lock(&ionum_mutex);
	pthread_mutex_lock(&worknum_mutex);
	pthread_mutex_lock(&clientsconn_mutex);
	pthread_mutex_lock(&inputworks_mutex);
	
	
	if (clients_connected > 0) {
		
		if ( iothreads_num < IOTHREADS_MAX && (float)iothreads_num/(float)clients_connected < 0.05 ) {
			pthread_create(&thread_id, NULL, IOThread, NULL);
			pthread_mutex_lock(&reg_mutex);
			sem_post(&reg_semaphore);
			
			iothreads_num++;
			
			pthread_mutex_unlock(&reg_mutex);
			
		}
		else if ( iothreads_num > IOTHREADS_MIN && (float)iothreads_num/(float)clients_connected > 1 ) {
			kill(getpid(),SIGUSR1);
			
		}
		
	}
	
	if (input_works > 0) {
		if ( workerthreads_num < WORKER_THREADS_MAX && (float) workerthreads_num/(float)input_works < 0.05 ) {
			pthread_create(&thread_id, NULL, workerThread, NULL);
			addSlot();
			
		}
		else if (  workerthreads_num > WORKER_THREADS_MIN && (float) workerthreads_num/(float)input_works > 1 ) {
			kill(getpid(), SIGUSR2);
		}
	}
	
	pthread_mutex_unlock(&ionum_mutex);
	pthread_mutex_unlock(&worknum_mutex);
	pthread_mutex_unlock(&clientsconn_mutex);
	pthread_mutex_unlock(&inputworks_mutex);
	
	#ifdef DEBUG
		printf("Thread %u: LoadAdaptation completato\n", (unsigned int)pthread_self());
	#endif
	
}
