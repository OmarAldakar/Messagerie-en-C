#ifndef CL_H
#define CL_H

#include "hashmap.h"

/**
 * @brief Structure correspondant aux éléments
 *        de la liste d'inondation
 *
 */
typedef struct m_list {
  hashmap* neighb;
  unsigned char msg[242];
  unsigned char id[8];
  unsigned char nonce[4];
  unsigned char type; // type du TLV Data
  unsigned char length; // taille du TLV Data
} m_list;

/**
 * @brief Structure correspondant à la liste d'inondation
 *
 */
typedef struct circ_list {
  m_list** msg_l; // la liste d'inondation
  int size; // la taille totale de la liste
  int index; // position dans la liste
} circ_list;

circ_list* init_clist(int size);

void add_msg(circ_list* c_list,hashmap* neighb,unsigned char msg[242],unsigned char id[8], unsigned char nonce[4],
             unsigned char type,unsigned char length);

void free_clist(circ_list* c_list);

void free_mlist(m_list* msg_l);

void delete_index(circ_list* c_list,int pos);

short exist(circ_list* c_list,unsigned char id[8],unsigned char nonce[4]);

m_list* get_from_clist(circ_list* c_list,unsigned char id[8],unsigned char nonce[4]);

void remove_neigbh(circ_list* c_list, unsigned char id[8],unsigned char nonce[4],unsigned char ip[16],unsigned short port);

void pr_circ_list(circ_list* c_list);
#endif
