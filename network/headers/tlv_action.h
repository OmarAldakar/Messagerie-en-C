#ifndef TLVA_H
#define TLVA_H

#include "pair.h"

short hello_action(pair* p,unsigned char* tampon, unsigned char sder_ip[16],unsigned short sder_port);

short neighbour_action(pair* p,unsigned char* tampon,unsigned char sndr_ip[16], unsigned short sndr_port);

short data_action(pair* p,unsigned char* tampon, unsigned char sndr_ip[16], unsigned short sndr_port);

short ack_action(pair* p,unsigned char* tampon, unsigned char sder_ip[16], unsigned short sder_port);

short goAway_action(pair* p,unsigned char* tampon, unsigned char sder_ip[16], unsigned short sder_port);

void warning_action(unsigned char* tampon);

#endif
