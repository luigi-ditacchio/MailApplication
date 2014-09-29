#ifndef _MAILBOX_
#define _MAILBOX_

#include <pthread.h>

#include "protocol.h"

typedef struct FileLock FileLock;
struct FileLock {
	int ds_file;
	char filename[MAX_NAME + 1];
	pthread_mutex_t mutex;
	short queue;
	FileLock * next;
};

FileLock * 		first_filelock;
pthread_mutex_t 	 filelock_mutex;


void initMailbox();
void addSlot();
void removeSlot();
FileLock * lockFile(char *name);
void unlockFile(FileLock * ptr);


#endif

