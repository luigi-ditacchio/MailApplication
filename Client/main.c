#include <Windows.h>
#include <WinSock.h>
#include "interface.h"
#include "systemAccess.h"
#include "protocol.h"

#include <stdio.h>  ////DA CANCELLARE!!! SOLO PER TEST!!!!

#define SERVER_PORT 6789
#define SERVER_NAME "192.168.1.101"

SOCKET ds_sock;


int main() {

	
	WSADATA wsadata;
	struct sockaddr_in server_addr;
	struct hostent *server_ip;
	AuthType type;
	SHORT result_opt;
	SHORT result_par;
	SHORT result_auth;
	CHAR name[20];
	CHAR psw[20];
	//SHORT auth_result;



	welcome();

/*********************************************************************
 **																	**
 **					INIZIALIZZAZIONE DEL SOCKET						**
 **																	**
 *********************************************************************/
	
	if (WSAStartup (MAKEWORD(2,2), &wsadata) != 0) {
		fatalError("Errore nell'inizializzazione dell'interfaccia Windows Socket");
	}

	ds_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (ds_sock == -1) {
		fatalError("Errore nell'installazione del socket");
	}

	memset((void *)&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);

	server_ip = gethostbyname(SERVER_NAME);
	memcpy((void *)&server_addr.sin_addr, (void *)server_ip->h_addr, 4);

	
	//if (connect(ds_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == -1) {
	//	fatalError("Errore nella connessione al server");
	//}
	
	connectionSucceeded();

/*********************************************************************
 **																	**
 **			INIZIO DELLE OPERAZIONI DI ACCESSO AL SERVIZIO			**
 **																	**
 *********************************************************************/
	do {

		result_opt = mainMenu();

		switch (result_opt) {

		case OPT_LOGIN:
			type = AUTH_LOGIN;
			break;

		case OPT_REGISTRATION:
			type = AUTH_REGISTRATION;
			break;

		case OPT_EXIT:
			ExitProcess(0);
			break;

		case OPT_ERROR:
			fatalError("Fallimento");
			ExitProcess(1);
			break;
		}
	
		result_par = getAccessParameters(name, psw, 20, 20);

		switch (result_par) {

		case PAR_OK:
			break;

		case PAR_EXIT:
			ExitProcess(0);
			break;

		case PAR_ERROR:
			fatalError("Fallimento");
			ExitProcess(1);
			break;
		}

		result_auth = authentication(ds_sock, name, psw, type);
	
		switch (result_auth) {

		case AUTH_SUCCEEDED:
			authorizationOk("");
			break;

		case AUTH_NO_SUCCEEDED:
			authorizationFailed();
			break;

		case AUTH_ERROR_SEND:
			fatalError("Errore nella send");
			ExitProcess(1);
			break;

		case AUTH_ERROR_RECV:
			fatalError("Errore nella recv");
			ExitProcess(1);
			break;

		case AUTH_ERROR_UNKNOWN:
			fatalError("Errore sconosciuto");
			ExitProcess(1);
			break;

		}

	} while (result_auth == AUTH_NO_SUCCEEDED);

	while (1) {}
	

	//WSACleanup();
}