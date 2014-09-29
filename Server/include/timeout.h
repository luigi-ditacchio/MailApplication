#ifndef _TIMEOUT_
#define _TIMEOUT_

#include <pthread.h>

#include "clientState.h"
#include "stateAndTimeout.h"



struct TimeoutNode {
	TimeoutNode * previous;
	ClientState * state;
	TimeoutNode * next;
};


TimeoutNode * first_timeout;	

pthread_mutex_t timeout_mutex;

void initTimeout();
void addNodeTimeout(ClientState * state);
void removeNodeTimeout(ClientState * state, unsigned char lock);
void timeout();


#endif
