#ifndef PAIR_H
#define PAIR_H

#include "../../library/headers/p_list.h"
#include "../../library/headers/hashmap.h"
#include "../../library/headers/circ_list.h"
#include "../../library/headers/big_msg_arr.h"
#include "pthread.h"

/**
 * @brief Structure pour le partage de structure entre les threads
 *
 */
typedef struct mutexes {
  pthread_mutex_t neighb_mutex;
  pthread_mutex_t p_neighb_mutex;
  pthread_mutex_t circ_l_mutex;
  pthread_mutex_t arr_mutex;
} mutexes;

/**
 * @brief Strucutre d'un pair (client/serveur)
 *
 */
typedef struct pair {
  char* pseudo;
  int socket;
  unsigned char id[8];
  hashmap* neighbours;
  p_list* p_neighb;
  circ_list* inondation_list;
  mutexes mutexs;
  bm_array* bm_arr;
} pair;


pair* create_pair(unsigned short port, char* pseudo);

short init_socket(pair* p,unsigned short port);

short addNeighb(pair* p, unsigned char id[8], unsigned char ip[16], short port);

short removeNeighb(pair* p, unsigned char ip[16],short port);

short addPotNeighbString(pair* p, char* ip, char* port);

short addPotNeighb(pair* p, unsigned char ip[16], short port);

short removePotNeighb(pair* p,unsigned char ip[16],short port);

short addMessageToIndList(pair* p, unsigned char id[8], unsigned char nonce[4], unsigned char message[4096], unsigned char type, unsigned char length);

void pr_pair(pair* p);

void cleanPotNeighb(pair* p);

void cleanNeighb(pair* p);

void cleanData(pair* p);

void add_bma(pair* p,unsigned char msg[242], unsigned char id[8],unsigned char len);

void update_bmdisplay(pair* p,unsigned char id[8],unsigned char msg[242]);

#endif
