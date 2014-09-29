#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mailbox.h"
#include "workerThread.h"


void initMailbox() {
	
	int i;
	FileLock * ptr;
	
	first_filelock = (FileLock *)malloc(sizeof(FileLock));
	ptr = first_filelock;
	ptr -> ds_file = -1;
	pthread_mutex_init( &(ptr -> mutex), NULL);
	ptr -> queue = 0;
	
	for (i=1; i<WORKER_THREADS_MIN; i++) {
		ptr -> next = (FileLock *)malloc(sizeof(FileLock));
		ptr = ptr -> next;
		ptr -> ds_file = -1;
		pthread_mutex_init( &(ptr -> mutex), NULL);
		ptr -> queue = 0;
	}
	
	pthread_mutex_init( &filelock_mutex, NULL);
	
}


void addSlot() {

	FileLock * temp;

	pthread_mutex_lock(&filelock_mutex);
	
	temp = first_filelock;
	
	first_filelock = (FileLock *)malloc(sizeof(FileLock));
	first_filelock -> ds_file = -1;
	first_filelock -> queue = 0;
	pthread_mutex_init( &(first_filelock -> mutex), NULL);
	first_filelock -> next = temp;
	
	pthread_mutex_unlock(&filelock_mutex);
	
}

void removeSlot() {

	FileLock temp, *ptr;
	
	pthread_mutex_lock(&filelock_mutex);
	
	temp.next = first_filelock;
	while (temp.next != NULL) {
		if ((temp.next)->ds_file == -1) {
			ptr = temp.next;
			temp.next = (temp.next)->next;
			pthread_mutex_destroy(&(ptr->mutex));
			free(ptr);
			break;
		}
	}
	first_filelock = temp.next;
	
	
	pthread_mutex_unlock(&filelock_mutex);
}


FileLock * lockFile(char *name) {
	
	FileLock * ptr;
	
	#ifdef DEBUG
		printf("Acquisizione lock su file %s\n", name);
	#endif
	pthread_mutex_lock(&filelock_mutex);
	
	ptr = first_filelock;
	
	//CONTROLLO CHE IL FILE NON SIA GIÀ APERTO
	while (ptr != NULL) {
		if (ptr->ds_file != -1) {
			if (strcmp(ptr->filename, name) == 0) {
				//IL FILE E' GIÀ APERTO
				ptr->queue += 1;
				pthread_mutex_unlock(&filelock_mutex);
				pthread_mutex_lock( &(ptr->mutex) );
				return ptr;
			}
		}
		ptr = ptr->next;
	}
	
	ptr = first_filelock;
	
	//IL FILE NON È GIÀ APERTO	
	while (ptr != NULL) {
		if (ptr->ds_file == -1) {
			ptr->ds_file = 0;
			ptr->queue = 1;
			strcpy( (char *)(ptr->filename), name);
			pthread_mutex_unlock(&filelock_mutex);
			pthread_mutex_lock( &(ptr->mutex) );
			return ptr;
		}
		ptr = ptr->next;
	}
	
	
	pthread_mutex_unlock(&filelock_mutex);
	return NULL;
	
}


void unlockFile(FileLock * ptr) {

	pthread_mutex_lock(&filelock_mutex);

	ptr->queue -= 1;

	if (ptr->queue == 0) {
		ptr->ds_file = -1;
		
	}
	
	pthread_mutex_unlock( &(ptr->mutex) );

	pthread_mutex_unlock(&filelock_mutex);	

}


