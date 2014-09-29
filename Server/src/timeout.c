#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "timeout.h"
#include "IOThread.h"

void initTimeout() {

	first_timeout = NULL;
	
	pthread_mutex_init(&timeout_mutex, NULL);
}

void addNodeTimeout(ClientState * state) {

	pthread_mutex_lock(&timeout_mutex);
	
	TimeoutNode * new_node = (TimeoutNode *)malloc(sizeof(TimeoutNode));
	new_node -> previous = NULL;
	new_node -> state = state;
	new_node -> next = first_timeout;
	if (first_timeout != NULL) {
		first_timeout -> previous = new_node;
	}
	first_timeout = new_node;
	
	state -> to_node = new_node;

	pthread_mutex_unlock(&timeout_mutex);	
}


void removeNodeTimeout(ClientState * state, unsigned char lock) {

	TimeoutNode * previous, * next;
	
	if (lock) pthread_mutex_lock(&timeout_mutex);
	
	previous = (state -> to_node) -> previous;
	next = (state -> to_node) -> next;
	
	if (previous != NULL)
		previous -> next = next;
	if (next != NULL)
		next -> previous = previous;
	
	if (previous == NULL)
		first_timeout = next;
	
	free(state -> to_node);
	
	if (lock) pthread_mutex_unlock(&timeout_mutex);
	
}

void timeout() {

	TimeoutNode * ptr, * help;
	
	pthread_mutex_lock(&timeout_mutex);
	
	ptr = first_timeout;
	while (ptr != NULL) {
		if ( (ptr -> state) -> alive ) {
			(ptr -> state) -> alive = 0;
			ptr = ptr -> next;
		}
		else {
			help = ptr -> next;
			closeAndFree( ptr -> state, 0);
			removeNodeTimeout(ptr -> state, 0);
			ptr = help;
		}
	}
	
	pthread_mutex_unlock(&timeout_mutex);
	
	#ifdef DEBUG
		printf("Thread %u: Completato Timeout\n", (unsigned int)pthread_self());
	#endif
}
	

