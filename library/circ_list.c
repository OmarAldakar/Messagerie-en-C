#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>

#include "headers/circ_list.h"

#define true 1
#define false 0

/**
 * @brief Crée une liste d'inondation
 *
 * @param size taille de la liste d'inondation
 * @return circ_list* une liste d'inondation
 */
circ_list* init_clist(int size) {
  circ_list* c_list = malloc(sizeof(circ_list));

  if (c_list == NULL)
    return NULL;

  c_list->msg_l = malloc(sizeof(m_list*)*size);

  if (c_list->msg_l == NULL)
    return NULL;

  c_list->size = size;
  c_list->index = 0;

  for(int i=0; i < c_list->size; i++) {
    c_list->msg_l[i] = NULL;
  }

  return c_list;
}

/**
 * @brief Vérifie si un identifiant (id,nonce) se trouve dans la liste d'inondation
 *
 * @param c_list la liste d'inondation
 * @param id
 * @param nonce
 * @return 1 si elle se trouve dans la liste, 0 sinon
 */
short exist(circ_list* c_list,unsigned char id[8],unsigned char nonce[4]) {
  for(int i=0; i < c_list->size; i++) {
   m_list* elem = c_list->msg_l[i];

   if (elem != NULL) {
     short same_id = memcmp(id, elem->id, 8) == 0;
     short same_nonce = memcmp(nonce, elem->nonce, 4) == 0;

     if (same_id && same_nonce)
       return true;
   }

  }

  return false;
}

/**
 * @brief Récupère le noeud d'identifisant (id,nonce)
 *
 * @param c_list la liste d'inondation
 * @param id
 * @param nonce
 * @return m_list* le noeud d'identifiant (id, nonce)
 */
m_list* get_from_clist(circ_list* c_list,unsigned char id[8],unsigned char nonce[4]) {
  for(int i=0; i < c_list->size; i++) {
    m_list* elem = c_list->msg_l[i];

    if (elem != NULL) {
      short same_id = memcmp(id, elem->id, 8) == 0;
      short same_nonce = memcmp(nonce, elem->nonce, 4) == 0;

      if (same_id && same_nonce)
	return elem;
    }
  }

  return false;
}

/**
 * @brief Ajoute un TLV Data à la liste d'inondation
 *
 * @param c_list la liste d'inondation
 * @param neighb les voisins symétriques de notre pair
 * @param msg le message contenu dans le TLV Data
 * @param id contenu dans le TLV Data
 * @param nonce contenu dans le TLV Data
 * @param type du TLV Data
 * @param length taille du message (msg)
 * @return void
 */
void add_msg(circ_list* c_list,hashmap* neighb, unsigned char msg[242],unsigned char id[8], unsigned char nonce[4],
             unsigned char type, unsigned char length){
  if (exist(c_list,id,nonce))
    return;

  m_list* msg_node = c_list->msg_l[c_list->index];

  if (msg_node != NULL) {
    delete_index(c_list, c_list->index);
  }

  msg_node = malloc(sizeof(m_list));

  if (msg_node == NULL) {
    return;
  }

  memcpy(msg_node->msg, msg, 242);
  memcpy(msg_node->id, id,8);
  memcpy(msg_node->nonce, nonce, 4);
  msg_node->neighb = copy_hashmap(neighb);
  msg_node->type = type;
  msg_node->length = length;
  c_list->msg_l[c_list->index] = msg_node;

  c_list->index = (c_list->index + 1) % c_list->size;
}

/**
 * @brief Fonction libérant l'espace d'un noeud de la liste
 *        d'inondation
 *
 * @param msg_l le noeud dont on veut libérer l'espace en mémoire
 * @return void
 */
void free_mlist(m_list* msg_l) {
  if (msg_l == NULL)
    return;

  delete_hashmap(msg_l->neighb);
  free(msg_l);
}

/**
 * @brief Fonction libérant l'espace d'un noeud de la liste
 *        d'inondation à une position précise
 *
 * @param c_list la liste d'inondation
 * @param pos position du noeud dont on veut libérer l'espace en mémoire
 * @return void
 */
void delete_index(circ_list* c_list,int pos) {
  free_mlist(c_list->msg_l[pos]);
  c_list->msg_l[pos] = NULL;
}

/**
 * @brief Fonction libérant l'espace totale de la liste d'inondation
 *
 * @param c_list la liste d'inondation
 * @return void
 */
void free_clist(circ_list* c_list) {
  for(int i=0; i < c_list->size; i++) {
    free_mlist(c_list->msg_l[i]);
  }
  free(c_list);
}

/**
 * @brief Enlever le voisin (ip, port) du noeud (id,nonce) de la liste d'inondation
 *
 * @param c_list la liste d'inondation
 * @param id
 * @param nonce
 * @param ip
 * @param port
 * @return void
 */
void remove_neigbh(circ_list* c_list, unsigned char id[8],unsigned char nonce[4],unsigned char ip[16],unsigned short port) {
  m_list* msg = get_from_clist(c_list, id, nonce);
  remove_from_hashmap(msg->neighb, ip, port);
}

// FONCTIONS D'AFFICHAGE //

void print_char(unsigned char tab[],int size) {
  for (int i=0; i < size; i++) {
    printf("%d ",tab[i]);
  }
  printf("\n");
}

void pr_circ_list(circ_list* c_list) {
  for (int i=0; i < c_list->size; i++) {
    m_list* msg = c_list->msg_l[i];
    if (msg != NULL) {
      printf("--------- MESSAGE %d -----------\n",i);
      printf("ID:\n");
      print_char(msg->id, 8);
      printf("NONCE:\n");
      print_char(msg->nonce, 4);
      printf("type : %d \n", msg->type);
      printf("SIZE : %d \n", msg->length);
      printf("%.*s \n",msg->length,msg->msg);
      pr_hashmap(msg->neighb);
    }
  }
}
