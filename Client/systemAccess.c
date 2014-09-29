#include <Windows.h>
#include "systemAccess.h"
#include "protocol.h"


SHORT authentication(SOCKET ds_sock, CHAR *name, CHAR *password, AuthType type) {

	AuthClientPacket packet;
	AuthServerPacket server_packet;
	INT total_sent, sent_now;
	INT total_recv, recv_now;
	
	switch 
	if (option == 1) {
		packet.code = LOG_IN;
	}
	else if (option == 2) {
		packet.code = REGISTRATION;
	}

	strcpy(packet.name, name);
	strcpy(packet.password, password);


	/********************************************************
	 *			INVIO DEL PACCHETTO DI AUTORIZZAZIONE		*
	 ********************************************************/

	total_sent = 0;
	do {
		sent_now = send(ds_sock, (CHAR *)&packet + total_sent, sizeof(AuthClientPacket)-total_sent, 0);
		if (sent_now < 0) {
			return AUTH_ERROR_SEND;
		}
		total_sent += sent_now;
	} while (sent_now != 0);


	/********************************************************
	 *			RICEZIONE DELLA RISPOSTA DEL SERVER 		*
	 ********************************************************/

	total_recv = 0;
	do {
		recv_now = recv(ds_sock, (CHAR *)&server_packet + total_recv, sizeof(AuthServerPacket)-total_recv, 0);
		if (recv_now < 0) {
			return AUTH_ERROR_RECV;
		}
		total_recv += recv_now;
	} while (recv_now != 0);


	/********************************************************
	 *					RESPONSO DEL SERVER			 		*
	 ********************************************************/

	switch (server_packet.code) {

	case AUTHORIZED: 
		return AUTH_SUCCEEDED;
		break;

	case NOT_AUTHORIZED:
		return AUTH_NOT_SUCCEEDED;
		break;

	default:
		return AUTH_ERROR_UNKNOWN;
		break;

	}

}