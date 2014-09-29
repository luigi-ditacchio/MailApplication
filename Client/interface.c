#include <stdio.h>
#include <Windows.h>
#include "interface.h"

VOID welcome() {
	
	printf("************************************************************************\
		  \n*                    BENVENUTO NEL CLIENT DI POSTA                     *\
		  \n************************************************************************\
		  \n\
		  \nInizializzazione e connessione al server in corso...\
		  \n");
}

VOID connectionSucceeded() {

	printf("\nConnessione con il server avvenuta!\
		   \n\
		   \n\
		   \nPer accedere al sistema e' necessario AUTENTICARSI\
		   \n\
		   \nVerranno percio' richiesti un nome utente e una password\
		   \n\
		   \nIn alternativa si puo' effettuare la registrazione creando un proprio account di posta\
		   \n");
}

SHORT mainMenu() {

	USHORT option;
	INT control;

	printf("\n_________________________________________________________\
		    \n|                                                        |\
			\n|                    MENU PRINCIPALE                     |\
			\n|________________________________________________________|\
			\n\
			\n\
			\n**********************************************************\
			\n*                                                        *\
			\n*  (1) Possiedo un account e voglio accedere al sistema  *\
			\n*  (2) Voglio effettuare una registrazione               *\
			\n*  (3) Voglio uscire dal programma                       *\
			\n*                                                        *\
			\n**********************************************************\
			\n\
		    \nDigitare il numero corrispondente all'operazione desiderata:\
		    \n");
	fflush(stdout);

	control = scanf_s("%u", &option, 1);
	while (control == 0 || control == EOF || (option != 1 && option != 2 && option!=3) ) {
		if (control == EOF) {
			fatalError("Il dispositivo e' stato disconnesso");
		}
		printf("\nOpzione non valida, digitare 1, 2 o 3:\
				\n");
		fflush(stdin);
		control = scanf_s("%u", &option, 1);
	}
	fflush(stdin);

	switch (option) {

	case 1:
		return OPT_LOGIN;
		break;

	case 2:
		return OPT_REGISTRATION;
		break;

	case 3:
		return OPT_EXIT;
		break;

	default:
		return OPT_ERROR;
		break;
	}

}

SHORT getAccessParameters(CHAR *name, CHAR *password, BYTE size_name, BYTE size_password) {

	INT control;
	USHORT option;

	/****************************************************
	 *				SCANSIONE DEL NOME					*
	 ****************************************************/
	do {
		printf("\n\
			   \nDigitare un nome utente (NON puo' contenere spazi, lunghezza max %u caratteri):\
			   \n", size_name);
		fflush(stdout);
	
		control = scanf_s("%s", name, size_name+1);
		while (control == 0 || control == EOF) {
			printf("\nNome utente non valido, inserire un nome utente valido:\
				   \n");
			fflush(stdin);
			control = scanf_s("%s", name, size_name+1);
		}
		fflush(stdin);
	
		printf("\nNome utente inserito: %s\
			   \n", name);

	/****************************************************
	 *			SCANSIONE DELLA PASSWORD				*
	 ****************************************************/
	
		printf("\n\
			   \nDigitare una password (NON puo' contenere spazi, lunghezza massima %u caratteri):\
			   \n", size_password);
		fflush(stdout);

		control = scanf_s("%s", password, size_password+1);
		while (control == 0 || control == EOF) {
			printf("\nPassword non valida, inserire una password valida:\
				   \n");
			fflush(stdin);
			control = scanf_s("%s", password, size_password+1);
		}
		fflush(stdin);
	
		printf("\nPassword inserita: %s\
			   \n", password);


	/****************************************************
	 *			RICAPITOLAZIONE INFORMAZIONI			*
	 ****************************************************/
	
		printf("\nRICAPITOLAZIONE INFORMAZIONI INSERITE\
			   \n\
			   \nNome utente: %s\
			   \nPassword: %s\
			   \n\
			   \nPremere 1 per confermare\
			  \nPremere 2 per reinserire un nome utente e una password\
			   \nPremere 3 per uscire dal programma\
			   \n", name, password);

		fflush(stdout);

		control = scanf_s("%u", &option, 1);
		while (control == 0 || control == EOF || (option != 1 && option != 2 && option!=3) ) {
			if (control == EOF) {
				fatalError("Il dispositivo e' stato disconnesso");
			}
			printf("\nOpzione non valida, digitare 1, 2 o 3:\
					\n");
			fflush(stdin);
			control = scanf_s("%u", &option, 1);
		}
		fflush(stdin);

	} while (option == 2);

	switch (option) {

	case 1:
		return PAR_OK;
		break;

	case 3:
		return PAR_EXIT;
		break;

	default:
		return PAR_ERROR;
		break;

}


VOID authorizationOK(CHAR *message) {

	
	printf("\n///////////////////////////////////////////\
		    \n/                                         /\
			\n/             ACCESSO RIUSCITO            /\
			\n/                                         /\
			\n///////////////////////////////////////////\
			\n\
			\nMESSAGGIO DAL SERVER\
			\n%s\
			\n", message);

	
}


VOID authorizationFailed(CHAR *message) {

	INT control;
	USHORT new_option;

	printf("\n///////////////////////////////////////////\
		    \n/                                         /\
			\n/             ACCESSO NEGATO              /\
			\n/                                         /\
			\n///////////////////////////////////////////\
			\n\
			\nMOTIVO DEL FALLIMENTO\
			\n%s\
			\n\
			\nSi verra' reindirizzati al menu principale\
			\n", message);

	fflush(stdout);

	control = scanf_s("%u", &new_option, 1);
	while (control == 0 || control == EOF || (option != 1 && option != 2 && option != 3) ) {
		if (control == EOF) {
			fatalError("Il dispositivo e' stato disconnesso");
		}
		printf("\nOpzione non valida, digitare 1 o 2:\
				\n");
		fflush(stdin);
		control = scanf_s("%u", &new_option, 1);
	}
	fflush(stdin);
	if (new_option == 3) {
		printf("\nIl programma verra' terminato\
			    \n\
				\nPremere un tasto per continuare\
				\n");
		CHAR any;
		fgets(&any, 1, stdin);
		ExitProcess(0);
	}
	return new_option;
}


VOID fatalError(LPCSTR error_mex) {
 
	CHAR any[1];

	fprintf(stderr,"///////////////////////////////////////////////\
		          \n/                                             /\
		          \n/       C'E' STATO UN ERRORE IMPREVISTO       /\
	        	  \n/                                             /\
		          \n///////////////////////////////////////////////\
		          \n\
		          \n%s\
		          \n\
		          \n\
		          \nIL PROGRAMMA VERRA' TERMINATO\
		          \n\
		          \n\
		          \n", error_mex);
	fgets(any, 1, stdin);
	ExitProcess(1);

}

 static void inputStringa(char * stringa, int maxLength) {

	int c;
	int i = 0;

	scanf("%c", &c);

	while (c != '\n') {
		if (i < maxLength) {
			stringa[i] = c;
			i++;
		}
		scanf("%c", &c);
	}
	stringa[i] = '\0';

}
