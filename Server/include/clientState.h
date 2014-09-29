#ifndef _CLIENTSTATE_
#define _CLIENTSTATE_

#include "protocol.h"
#include "timeout.h"
#include "stateAndTimeout.h"

typedef struct {
	char * data;
	int size;
} WriteBlock;


struct ClientState {
	int ds_sock;
	unsigned char authenticated;
	char *name;
	unsigned char alive;
	WriteBlock * wr_block;
	TimeoutNode * to_node;
};


#endif
