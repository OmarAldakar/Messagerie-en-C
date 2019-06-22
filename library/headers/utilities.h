#ifndef UTIL_H
#define UTIL_H

void gen_random(unsigned char* data,int size);

short resolve_host(char* ip, char* port, unsigned char ip_stock[16], short* port_s);

int min(int a, int b);

void display_msg_shell(unsigned char* tampon, int tlv_length);

#endif
