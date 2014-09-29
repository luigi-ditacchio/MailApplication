#include <stdlib.h>
#include <stdio.h>

#include "interThreadsCommunication.h"

void initThreadsCommunication() {

	pthread_mutex_init(&input_mutex, NULL);
	pthread_mutex_init(&output_mutex, NULL);
	pthread_mutex_init(&iothreads_mutex, NULL);
	
	pthread_cond_init(&input_ready, NULL);
	
	first_input = NULL;
	first_output = NULL;
	last_input = NULL;
	last_output = NULL;
}

void getInput(ClientState * * state, void * * data, int * size) {

	IONode * node_ptr;
	
	pthread_mutex_lock(&input_mutex);
	
	while (first_input == NULL) {
		#ifdef DEBUG
			printf("Thread %u: Nessuna richiesta, mi metto in attesa\n", (unsigned int)pthread_self());
		#endif
		pthread_cond_wait( &input_ready, &input_mutex);
	}
	
	#ifdef DEBUG
		printf("Thread %u: Nuova richiesta, processamento in corso\n", (unsigned int)pthread_self());
	#endif
	
	node_ptr = first_input;
	first_input = first_input -> next;
	
	pthread_mutex_unlock(&input_mutex);
	
	*state = node_ptr -> state;
	*data = node_ptr -> data;
	*size = node_ptr -> size;
	
	free(node_ptr);
	
	//Decremento variabile lavoro in input
	pthread_mutex_lock(&inputworks_mutex);
	input_works--;
	pthread_mutex_unlock(&inputworks_mutex);
	
	#ifdef DEBUG
		printf("Thread %u: Terminato processamento\n", (unsigned int)pthread_self());
	#endif
	
	return;
	
}

void addInput(ClientState * state, void * data, int size) {

	IONode * node_ptr = malloc(sizeof(IONode));
	
	node_ptr -> state = state;
	node_ptr -> data = data;
	node_ptr -> size = size;
	node_ptr -> next = NULL;
	
	pthread_mutex_lock(&input_mutex);
				
	if (first_input == NULL) {
		first_input = node_ptr;
		last_input = first_input;
	}
	else  {
		last_input -> next = node_ptr;
		last_input = last_input -> next;
	}


	pthread_cond_signal(&input_ready);

	pthread_mutex_unlock(&input_mutex);
	
	//Incremento variabile lavoro in input
	pthread_mutex_lock(&inputworks_mutex);
	input_works++;
	pthread_mutex_unlock(&inputworks_mutex);
	
	return;

}

unsigned char getOutput(ClientState * * state, void * * data, int * size) {

	IONode * node_ptr;
	
	if ( pthread_mutex_trylock(&iothreads_mutex) == 0) {
		#ifdef DEBUG
			printf("Thread %u: Preso lock, controllo coda di output\n", (unsigned int)pthread_self());
		#endif
		pthread_mutex_lock(&output_mutex);
		
		if (first_output != NULL) {
			#ifdef DEBUG
				printf("Thread %u: Invio risposta sul socket %d\n", (unsigned int)pthread_self(), (first_output -> state) -> ds_sock );
			#endif
			node_ptr = first_output;
			first_output = first_output -> next;
			
			pthread_mutex_unlock(&output_mutex);
			pthread_mutex_unlock(&iothreads_mutex);
			
			*state = node_ptr -> state;
			*data = node_ptr -> data;
			*size = node_ptr -> size;
			
			free(node_ptr);
			
			return 1;
		}
		pthread_mutex_unlock(&output_mutex);
		pthread_mutex_unlock(&iothreads_mutex);
	}
	return 0;
	
}

void addOutput(ClientState * state, void * data, int size) {

	IONode * node_ptr = malloc(sizeof(IONode));
	
	node_ptr -> state = state;
	node_ptr -> data = data;
	node_ptr -> size = size;
	node_ptr -> next = NULL;
	
	pthread_mutex_lock(&output_mutex);
				
	if (first_output == NULL) {
		first_output = node_ptr;
		last_output = first_output;
	}
	else  {
		last_output -> next = node_ptr;
		last_output = last_output -> next;
	}


	pthread_mutex_unlock(&output_mutex);
	
	return;

}


void addOutputCode(ClientState * state, ReplyCode code) {

	ReplyCode * data = (ReplyCode *)malloc(sizeof(ReplyCode));
	*data = code;
	
	addOutput(state, data, sizeof(ReplyCode));
	
}

void addInputCode(ClientState * state, ReqCode code) {

	ReqCode * data = (ReqCode *)malloc(sizeof(ReqCode));
	*data = code;
	
	addInput(state, data, sizeof(ReqCode));
	
} 
