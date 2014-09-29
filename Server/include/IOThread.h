#ifndef _IOTHREAD_
#define _IOTHREAD_

#include <pthread.h>
#include "clientState.h"

#define IOTHREADS_MIN  3
#define IOTHREADS_MAX  10

int               ds_epoll;		//descrittore della struttura epoll
pthread_mutex_t   onlyone_mutex;	//mutex per l'accesso esclusivo da parte degli IOThreads alla lista di OutputNode

int               iothreads_num;
pthread_mutex_t   ionum_mutex;

void * IOThread(void * arg);
void closeAndFree( ClientState * state, unsigned char timeout);

#endif
