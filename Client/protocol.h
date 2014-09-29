/***********************************************************************************************************/
/*																										   */
/*			QUESTO HEADER DEFINISCE IL FORMATO DEI MESSAGGI SCAMBIATI TRA CLIENT E SERVER				   */
/*																										   */
/***********************************************************************************************************/

#ifndef _PROTOCOL_
#define _PROTOCOL_
//////////////////////////////////////////////////////////////////////////
/////////////////////////CODICI DI COMUNICAZIONE/////////////////////////
////////////////////////////////////////////////////////////////////////

/////Pacchetti mandati dal client
#define LOG_REQ_CODE 101
#define REG_REQ_CODE 102

/////Pacchetti mandati dal server
#define AUTH_OK_CODE     301
#define AUTH_ERROR_CODE  401

//////////////////////////////////////////////////////////////////////////
//////////////////////////TAGLIE DEI CAMPI///////////////////////////////
////////////////////////////////////////////////////////////////////////

#define MAX_NAME_LENGTH 21
#define MAX_PSW_LENGTH 21
#define MAX_REPLY_AUTH 31

//////////////////////////////////////////////////////////////////////////
///////////////////////////FASE DI AUTORIZZAZIONE////////////////////////
////////////////////////////////////////////////////////////////////////
typedef enum {
	LOG_IN = LOG_REQ_CODE,
	REGISTRATION = REG_REQ_CODE
} AuthClientCode;

typedef enum {
	AUTHORIZED = AUTH_OK_CODE,
	NOT_AUTHORIZED = AUTH_ERROR_CODE
} AuthServerCode;



typedef struct {
	AuthClientCode code;
	CHAR name[MAX_NAME_LENGTH];
	CHAR password[MAX_PSW_LENGTH];
} AuthClientPacket;

typedef struct {
	AuthServerCode code;
	CHAR message[MAX_REPLY_AUTH];
} AuthServerPacket;


#endif