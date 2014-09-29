#ifndef _PROTOCOL_
#define _PROTOCOL_

/************************************************************************************************
 *												                                                *
 *	QUESTO HEADER DEFINISCE IL FORMATO DEI MESSAGGI SCAMBIATI TRA CLIENT E SERVER	         	*
 *											                                                 	*
 ************************************************************************************************/

//////////////////////////////////////////////////////////////////////////
//////////////////////////MESSAGGI DEL CLIENT/////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*
			PACCHETTO DI AUTORIZZAZIONE (LOGIN O REGISTRAZIONE)

     ________________ ________________ ______ ________________ ______ 
	|		         |	              |	     |	              |      | 
	|    AuthCode    |	   Name	      |  \0  |	    Psw	      |  \0  |
	|________________|________________|______|________________|______|

			4	       4 min - 20 max     1    6 min - 20 max     1   
		



			PACCHETTO DI RICHIESTA DEL SERVIZIO: RICEVI TUTTI I MESSAGGI / CANCELLA TUTTI I MESSAGGI

															
													
											 _______________
											|				|
											|	 OpCode	    |
											|_______________|
													4			
				


							 PACCHETTO DI RICHIESTA DEL SERVIZIO: INVIA UN MESSAGGIO
						
	  ________________ ________	________________ ______ __________________ ______ ____________________________
	 |				  |		   |                |	   |	              |      |                     |      |
	 |      From	  |   \0   |	   To	    |  \0  |     Subject	  |  \0  |        Body         |  \0  |
	 |________________|________|________________|______|__________________|______|_____________________|______|

	  "From:" + 20 max    1      "To:" + 20 max    1    "Subject:" + 20 max   1          1000 max          1

*/

//////////////////////////////////////////////////////////////////////////
//////////////////////////MESSAGGI DEL SERVER/////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*
	
	
	PACCHETTO DI RISPOSTA: INVIA UN MESSAGGIO / RIMUOVI TUTTI I MESSAGGI

							 ________________ 
							|		         |  
							|    ReplyCode   |       
							|________________|

									4	     
		

	PACCHETTO DI RISPOSTA: RICEVI TUTTI I MESSAGGI


	 ________________ ________ ________
	|                |        |        |
	|      Body      |  \r\n  |  \r\n  |
	|________________|________|________|
		

*/
//////////////////////////////////////////////////////////////////////////
//////////////////////////TAGLIE DEI CAMPI////////////////////////////////
//////////////////////////////////////////////////////////////////////////
  
#define MAX_NAME         20
#define MIN_NAME          4
#define MAX_PSW          20
#define MIN_PSW           6
#define MAX_AUTH	MAX_NAME + MAX_PSW + 6
#define MIN_AUTH	MIN_NAME + MIN_PSW + 6

#define MAX_SUBJECT      20
#define MAX_BODY       1000
#define MIN_MAIL		 20
#define MAX_MAIL		2*MAX_NAME + MAX_SUBJECT + MAX_BODY + 20




typedef enum {
	LOGIN_REQ = 101,
	REG_REQ = 102,
	SEND = 831,
	RECV = 832,
	RMV = 833
} ReqCode;



typedef enum {

	
	LOGIN_OK = 201,
	LOGIN_ERROR = 211,
	
	REG_OK = 202,
	REG_ERROR = 212,
	
	INVALID_FORMAT = 230,
	
	OP_OK = 901,
	INVALID_SENDER = 244,
	INVALID_RECEIVER = 245,
	INVALID_MAIL = 912,
	
	INVALID_AUTHCODE = 990,
	INTERNAL_ERROR = 999
} ReplyCode;




#endif
