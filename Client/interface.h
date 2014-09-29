#ifndef _INTERFACE_
#define _INTERFACE_

/**********************************************************************
 *						FUNZIONE WELCOME()							  *
 *			      Stampa un messaggio di benvenuto			          *
 **********************************************************************/
VOID welcome();


/**********************************************************************
 *					FUNZIONE CONNECTIONSUCCEEDED()					  *
 *			 Informa dell'avvenuta connessione con il server e        *
 *				  delle modalità di accesso al servizio	              *
 **********************************************************************/
VOID connectionSucceeded();


/*****************************************************************************
 *							FUNZIONE MAINMENU()					             *
 *			    Mostra il menu principale con le varie opzioni               *
 *  (sotto alla segnatura sono indicati i valori di ritorno della funzione)  *
 *****************************************************************************/
SHORT mainMenu();
#define OPT_LOGIN         1
#define OPT_REGISTRATION  2
#define OPT_EXIT          3
#define OPT_ERROR        -1



/*****************************************************************************
 *					  FUNZIONE GETACCESSPARAMETERS()					     *
 *			  Prende in input dall'utente un nome e una password             *
 *				  necessari per l'accesso o la registrazione                 *
 *  (sotto alla segnatura sono indicati i valori di ritorno della funzione)  *
 *****************************************************************************/
SHORT getAccessParameters(CHAR *, CHAR *, BYTE, BYTE);
#define PAR_OK     1
#define PAR_EXIT   3
#define PAR_ERROR -1



/*****************************************************************************
 *					  FUNZIONE AUTHORIZATIONFAILED()					     *
 *			  Informa l'utente che l'autorizzazione non è riuscita			 *
 *						e ritorna al menu principale						 *
 *****************************************************************************/
VOID authorizationFailed();



/*****************************************************************************
 *					  FUNZIONE AUTHORIZATIONOK()		    			     *
 *			  Informa l'utente che l'autorizzazione è riuscita		     	 *
 *****************************************************************************/
VOID authorizationOk();


/*****************************************************************************
 *							 FUNZIONE FATALERROR()		    			     *
 *			  Informa l'utente che è avvenuto un errore imprevisto			 *
 *					che causerà la terminazione del programma				 *
 *****************************************************************************/
VOID fatalError(LPCSTR);

#endif