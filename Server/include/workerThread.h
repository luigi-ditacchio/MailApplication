#ifndef _WORKERTHREAD_
#define _WORKERTHREAD_

#define WORKER_THREADS_MIN	3
#define WORKER_THREADS_MAX	10

int	 		workerthreads_num;
pthread_mutex_t		worknum_mutex;

void * workerThread(void * arg);

#endif
