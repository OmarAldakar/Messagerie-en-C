#ifndef TRANS_H
#define TRANS_H

#include "../../library/headers/p_list.h"
#include "../../library/headers/hashmap.h"
#include "pair.h"

#define MTU 1024
#define NEIGBH_FREQ 10 //TOUTE LES COMBIEN DE SECONDE
#define SHELLO_FREQ 10 //TOUTE LES COMBIEN DE SECONDE

short send_request(pair* p, unsigned char ip[16], unsigned short port, int r_size, unsigned char request[MTU]);

void concat(unsigned char request[MTU],int i, short new_size, char* tlv,int tlv_len);

void init_header(unsigned char* request);

short sendTLVShortHello(pair* p);

short sendTLVAck(pair* p,unsigned char id[8], unsigned char nonce[4], unsigned char ip[16], short port);

short sendTLVGoAway(pair* p, int code,char* msg, unsigned char ip[16], short port);

short sendTLVWarning(pair* p, char* msg, unsigned char ip[16], short port);

short sendTLVData (pair* p, char* msg);

#endif
