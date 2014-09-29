#ifndef _SYSTEM_ACCESS_
#define _SYSTEM_ACCESS_

typedef enum {
	AUTH_LOGIN,
	AUTH_REGISTRATION
} AuthType;

SHORT authentication(SOCKET, CHAR *, CHAR *, AuthType);

/**********************************************************************
 *         VALORI DI RITORNO DELLA FUNZIONE AUTHENTICATION()          *
 **********************************************************************/
#define AUTH_SUCCEEDED        0        ///L'autorizzazione è andata a buon fine
#define AUTH_NO_SUCCEEDED    -1        ///L'autorizzazione non è andata a buon fine
#define AUTH_ERROR_SEND     -10        ///Errore nell'invio del pacchetto di autorizzazione al server
#define AUTH_ERROR_RECV     -11        ///Errore nella ricezione del pacchetto di risposta dal server
#define AUTH_ERROR_UNKNOWN  -20        ///Pacchetto ricevuto dal server sconosciuto

#endif