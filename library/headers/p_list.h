#ifndef PL_H
#define PL_H

#include <time.h>

/**
 * @brief Structure correspondant à un noeud de la
 *        liste de voisins potentiels
 *
 */
typedef struct p_neighb {
  unsigned char ip[16];
  unsigned short port;
  time_t first_contact; // premier TLV Hello Short envoyé
  time_t last_contact; // dernier TLV Hello Long envoyé
} p_neighb;

/**
 * @brief Structure correspondant à liste
 *        de voisins potentiels
 *
 */
typedef struct p_list  {
  p_neighb p_nei;
  struct p_list* next;
} p_list;

p_list* new_list();

p_list* add(p_list* list, unsigned char ip[16],unsigned short port);

p_list* delete(p_list* list,unsigned char ip[16],unsigned short port);

void pr_list(p_list* list);

#endif
