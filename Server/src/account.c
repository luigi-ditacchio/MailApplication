#include <sys/mman.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "account.h"
#include "workerThread.h"


char searchInReg(Entry * e, unsigned int * index);
char searchInNewReg(Entry * e, TreeNode * * last_node);
void addTreeNode(Entry * contact, unsigned int index, TreeNode * node);
void restructuring();
void writeOnTempReg(int ds, TreeNode * node, int * offset);
void buildTree(int ds);
void printTree();


void initAccount() {
	
	struct stat stat_reg;
	
	addr_reg = NULL;
	
	ds_reg = open("./data/registrations.rtf", O_RDWR | O_CREAT, 0666);
	if (ds_reg == -1) {
		printf("Errore nell'apertura del file degli utenti\n");
		exit(1);
	}
		
	fstat(ds_reg, &stat_reg);
	if (stat_reg.st_size > 0) {
		addr_reg = mmap(0, stat_reg.st_size, PROT_READ,
				 MAP_PRIVATE, ds_reg, 0);
		if (addr_reg == MAP_FAILED) {
			printf("Errore nel mapping di registrations\n");
			exit(1);
		}
	}
	reg_number = (stat_reg.st_size)/sizeof(Entry);
	
	
	ds_newreg = open("./data/new_registrations.rtf", O_RDWR | O_CREAT, 0666);
	if (ds_newreg == -1) {
		printf("Errore nell'apertura del file degli ultimi utenti registrati\n");
		exit(1);
	}
	fstat(ds_newreg, &stat_reg);
	newreg_number = (stat_reg.st_size)/sizeof(Entry);
	
	
	
	pthread_mutex_init(&reg_mutex, NULL);
	sem_init(&reg_semaphore, 0, WORKER_THREADS_MIN);
	sem_init(&newreg_semaphore, 0, WORKER_THREADS_MIN);
	
	if (newreg_number > 0) {
		buildTree(ds_newreg);
		restructuring();
	}
}


char login(char *name, char *psw) {

	Entry contact = {' '};
	
	//Preparazione entry contatto
	strcpy(contact.name, name);
	if (psw != NULL) {
		strcpy(contact.psw, psw);
	}
	contact.newline = '\n';
	
	
	//Rimozione gettoni da semafori lettura
	while (  sem_wait(&reg_semaphore)  == -1) {
		if (errno!=EINTR) {
			printf("Errore sconosciuto nella sem_wait\n");
			exit(1);
		}
	}
	while (  sem_wait(&newreg_semaphore)  == -1) {
		if (errno!=EINTR) {
			printf("Errore sconosciuto nella sem_wait\n");
			exit(1);
		}
	}
	
	//Ricerca nei due files
	if(searchInReg(&contact, NULL) == 1 || searchInNewReg(&contact, NULL) == 1) {
		sem_post(&reg_semaphore);
		sem_post(&newreg_semaphore);
		return 1;
	}
	else {
		sem_post(&reg_semaphore);
		sem_post(&newreg_semaphore);
		return 0;
	}
	
}
	


char registration(char *name, char *psw) {
	
	
	Entry contact = {' '};
	unsigned int index;
	TreeNode * node;
	int i;
	
	//Preparazione entry contatto
	strcpy(contact.name, name);
	strcpy(contact.psw, psw);
	contact.newline = '\n';
	
	//Acquisisco il lock su registrations
	pthread_mutex_lock(&reg_mutex);

	
	for (i=0; i<workerthreads_num; i++) {
		while (  sem_wait(&reg_semaphore)  == -1) {
			if (errno!=EINTR) {
				printf("Errore sconosciuto nella sem_wait\n");
				exit(1);
			}
		}
	}
	
	
	//Acquisisco il lock su new_registrations
	for (i=0; i<workerthreads_num; i++) {
		while (  sem_wait(&newreg_semaphore)  == -1) {
			if (errno!=EINTR) {
				printf("Errore sconosciuto nella sem_wait\n");
				exit(1);
			}
		}
	}

	
	
	//Cerco nei due files
	if (searchInReg(&contact, &index) != -1 || searchInNewReg(&contact, &node) != -1) {
	
		pthread_mutex_unlock(&reg_mutex);
		//Rimetto gettoni per lettura
		for (i=0; i<workerthreads_num; i++) {
			sem_post(&reg_semaphore);
		}
		//Rimetto gettoni per lettura
		for (i=0; i<workerthreads_num; i++) {
			sem_post(&newreg_semaphore);
		}
		return 0;
	}
	#ifdef DEBUG
		printf("Account corretto: non esiste né in registrations né in new_registrations\n");
	#endif
	
	while ( write(ds_newreg, (void *)&contact, sizeof(Entry) ) == -1) {
		if (errno != EINTR) {
			printf("Errore sconosciuto nella write su new_registrations\n");
			exit(1);
		}
	}
	#ifdef DEBUG
		printf("Account scritto in new_reg\n");
	#endif
		
	newreg_number++;
	
	addTreeNode(&contact, index, node);
	printTree("", root_node);
	
	if (newreg_number == MAX_NEWREG) {
		#ifdef DEBUG
			printf("Unione contatti registrations e new_registrations\n");
		#endif
		restructuring();
	}
	
	pthread_mutex_unlock(&reg_mutex);
	//Rimetto gettoni per lettura
	for (i=0; i<workerthreads_num; i++) {
		sem_post(&reg_semaphore);
	}
	//Rimetto gettoni per lettura
	for (i=0; i<workerthreads_num; i++) {
		sem_post(&newreg_semaphore);
	}
	
	return 1;
	
}



char searchInReg(Entry * e, unsigned int * index) {


	int first, last, mid;
	int cmp;
	
	if (reg_number == 0) {
		if (index != NULL) *index = 0;
		
		return -1;
	}
		
	first = 0;
	last = reg_number-1;
	
	while (first <= last) {
		mid = (first + last)/2;
		cmp = strcmp(e->name, ((Entry *)addr_reg + mid)->name);
		if (cmp == 0) {
			if (e->psw != NULL && strcmp(e -> psw, ((Entry *)addr_reg + mid)->psw)  == 0 ) {
				
				return 1;
			}
			else {
				
				return 0;
			}
		}
		if (cmp > 0)
			first = mid + 1;
		else last = mid - 1;
		
	}
	
	if (index != NULL) *index = first;
	
	return -1;

	
	
}




char searchInNewReg(Entry * e, TreeNode * * last_node) {

	TreeNode * ptr;
	int cmp;
	
	
	if(newreg_number == 0) {
		if (last_node != NULL)  *last_node = NULL;
		return -1;
	}
	
	//Inizializzazione variabili
	ptr = root_node;
	
	while (ptr != NULL) {
		cmp = strcmp(e->name, (ptr->entry).name);
		if(cmp==0) {
			if (e->psw != NULL && strcmp(e -> psw, (ptr->entry).psw)  == 0 ) {
				
				return 1;
			}
			else {
				
				return 0;
			}
		}
		else if (cmp > 0 && ptr->right_child != NULL) ptr = ptr->right_child;
		else if (cmp < 0 && ptr->left_child != NULL) ptr = ptr->left_child;
		else break;
	}
	
	if (last_node != NULL) *last_node = ptr;
	return -1;
		
}




void addTreeNode(Entry * contact, unsigned int index, TreeNode * node) {

	TreeNode * temp;

	temp = (TreeNode *)malloc(sizeof(TreeNode));
	memcpy( &(temp -> entry), (void *)contact, sizeof(Entry));
	temp->index = index;
	temp->left_child = NULL;
	temp->right_child = NULL;
	
	if (node == NULL)  root_node = temp;
	
	else {
		if ( strcmp(contact->name, (node->entry).name) > 0)  node -> right_child = temp;
		else  node -> left_child = temp;
	}
	
	#ifdef DEBUG
		printf("Dettagli nodo: Nome: %s, Password: %s, Index: %d\n", contact->name, contact->psw, index);
	#endif

	
}


void restructuring() {

	int partial = 0, tot = 0;

	int offset = 0;
	int ds = open("./data/temp.rtf", O_RDWR | O_CREAT, 0666);
	if (ds == -1) {
		printf("Errore nell'apertura/creazione di temp\n");
		raise(SIGTERM);
	}
	writeOnTempReg(ds, root_node, &offset);
	if (addr_reg != NULL) {
		do {
			partial = write(ds, addr_reg + (offset * sizeof(Entry)) + tot, (reg_number - offset)*sizeof(Entry) - tot);
			if (partial > 0) tot += partial;
			else if (partial == -1 && errno != EINTR) {
				printf("Errore sconosciuto nella write su temp_reg\n");
				exit(1);
			}
		} while ( (reg_number - offset)*sizeof(Entry) - tot > 0 );
	}
	fsync(ds);
	

	
	if (addr_reg != NULL) munmap(addr_reg, reg_number);
	
	//Reimposto i descrittori dei files
	while ( close(ds_reg) == -1 && errno==EINTR) {}
	if (rename("./data/temp.rtf", "./data/registrations.rtf") == -1) {
		printf("Errore nella sovrascrittura di registrations\n");
		raise(SIGTERM);
	}
	ds_reg = ds;
	reg_number += newreg_number;
	
	while ( close(ds_newreg) == -1 && errno==EINTR) {}
	ds_newreg = open("./data/new_registrations.rtf", O_RDWR | O_TRUNC);
	if (ds_newreg == -1) {
		printf("Errore nel troncamento di new_reg\n");
		raise(SIGTERM);
	}
	newreg_number = 0;
	
	//Reimposto le strutture
	root_node = NULL;
	addr_reg = mmap(0, reg_number*sizeof(Entry), PROT_READ, MAP_PRIVATE, ds_reg, 0);
	if (addr_reg == MAP_FAILED) {
		printf("Errore nel mapping di registrations\n");
		raise(SIGTERM);
	}
	

	
	#ifdef DEBUG
		printf("Ristrutturazione completata\n");
	#endif
	
}


void writeOnTempReg(int ds, TreeNode * node, int * offset) {
	
	int tot = 0;
	int partial = 0;
	
	if (node == NULL) return;
	
	writeOnTempReg(ds, node->left_child, offset);
	
	if (addr_reg != NULL) {
		do {
			partial = write(ds, addr_reg + (*offset * sizeof(Entry)) + tot, (node->index - *offset)*sizeof(Entry) - tot);
			if (partial > 0) tot += partial;
			else if (partial == -1 && errno != EINTR) {
				printf("Errore sconosciuto nella write su temp_reg\n");
				exit(1);
			}
		} while ( (node->index - *offset)*sizeof(Entry) - tot > 0 );
	}
	
	*offset = node->index;
	
	while ( write(ds, (void *)&(node -> entry), sizeof(Entry) ) == -1) {
		if (errno != EINTR) {
			printf("Errore sconosciuto nella write su new_registrations\n");
			exit(1);
		}
	}
	
	writeOnTempReg(ds, node->right_child, offset);
	
	free(node);
	
}


void buildTree(int ds) {

	Entry contact;
	int i, partial_r;
	unsigned int index;
	TreeNode * node;
	
	for(i=0; i<newreg_number; i++) {
		while ( partial_r = read(ds, (void *)&contact, sizeof(Entry)) == -1 && errno == EINTR) {}
		if (partial_r == -1) {
			printf("Errore nella costruzione dell'albero\n");
			exit(1);
		}
		searchInReg(&contact, &index);
		searchInNewReg(&contact, &node);
		addTreeNode(&contact, index, node);
		#ifdef DEBUG
			printf("Aggiunto nodo all'albero\n");
		#endif
	}
	#ifdef DEBUG
		printf("Terminata costruzione dell'albero\n");
	#endif
}

void printTree(char * s, TreeNode * node) {
	
	if (node == NULL) return;
	printf("%s", s);
	printf("%s\n", (node->entry).name);
	
	printTree("	", node->right_child);
	printTree("	", node->left_child);
	
}
