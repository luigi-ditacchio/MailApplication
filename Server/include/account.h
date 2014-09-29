#ifndef _ACCOUNT_
#define _ACCOUNT_

#include <pthread.h>
#include <semaphore.h>

#include "protocol.h"

#define MAX_NEWREG   3

typedef struct {
	char name[MAX_NAME + 1];
	char psw[MAX_PSW + 1];
	char newline;
} Entry;

typedef struct TreeNode TreeNode;
struct TreeNode {
	Entry entry;
	unsigned int index;
	TreeNode * left_child;
	TreeNode * right_child;
};

/*****************************************
 *      VARIABILI FILE REGISTRATIONS     *
 ****************************************/
int               reg_number;
pthread_mutex_t   reg_mutex;        //per scrittura
sem_t             reg_semaphore;    //per lettura
int               ds_reg;
void *            addr_reg;

/*****************************************
 *    VARIABILI FILE NEW REGISTRATIONS   *
 ****************************************/
int               newreg_number;
sem_t             newreg_semaphore;    //per lettura
int               ds_newreg;
TreeNode *        root_node;    


void initAccount();
char login(char *name, char *psw);
char registration(char *name, char *psw);


#endif
