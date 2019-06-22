#ifndef TLVM_H
#define TLVM_H

char* tlv_HelloShort(unsigned char sender[8]);

char* tlv_HelloLong(unsigned char sender[8], unsigned char recv[8]);

char* tlv_Neighbour(unsigned char ip[16], unsigned short port);

char* tlv_Data(unsigned char sender[8],char* msg,unsigned char msg_size, unsigned char type);

char* tlv_Ack(unsigned char sender[8], unsigned char nonce[4]);

char* tlv_GoAway(int code, char* msg, unsigned char length);

char* tlv_Warning(char* msg, unsigned char length);

char* tlv_big_data(unsigned char sender[8], unsigned short total, char msg[233],
		   unsigned short num, unsigned char nonce[4],unsigned char msg_size);

#endif
