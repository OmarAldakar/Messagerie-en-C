#ifndef REC_H
#define REC_H
#define SIZE 4096

#include "pair.h"

short receiveTLV(pair* p);

void analyseTLV(pair* p ,unsigned char* tampon, unsigned char sder_ip[16], unsigned short port);

#endif
