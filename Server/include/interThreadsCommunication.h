#ifndef _INTERTHREADCOMMUNICATION_
#define _INTERTHREADCOMMUNICATION_

#include <pthread.h>
#include "clientState.h"

	
typedef struct IONode IONode;
struct IONode {
	ClientState * state;
	void * data;
	int size;
	IONode * next;
};


IONode     	    * first_input, * last_input, * first_output, * last_output;
pthread_mutex_t     input_mutex, output_mutex, iothreads_mutex;
pthread_cond_t      input_ready;


int 			input_works;
pthread_mutex_t		inputworks_mutex;

void initThreadsCommunication();
void getInput(ClientState * * state, void * * data, int * size);
void addInput(ClientState * state, void * data, int size);
unsigned char getOutput(ClientState * * state, void * * data, int * size);
void addOutput(ClientState * state, void * data, int size);
void addOutputCode(ClientState * state, ReplyCode code);
void addInputCode(ClientState * state, ReqCode code);




#endif
