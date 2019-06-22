#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headers/big_msg_arr.h"


/**
 * @brief Crée un tableau qui stocke les gros messages (TLV Data type 220)
 *
 * @param size la taille du tableau
 *
 * @return un tableau vide
 */
bm_array* init_bma(int size) {
  bm_array* c_list = malloc(sizeof(bm_array));
  if (c_list == NULL)
    return NULL;

  c_list->circ_list = malloc(sizeof(bm_node*)*size);
  if (c_list->circ_list == NULL)
    return NULL;

  c_list->size = size;
  c_list->index = 0;
  for(int i=0; i < c_list->size; i++) {
    c_list->circ_list[i] = NULL;
  }

  return c_list;
}

/**
 * @brief Crée un nouvel élément du tableau de gros messages
 *
 * @param total_size la taille total du message global
 * @param id l'id du message global
 * @param nonce la nonce du message global
 * @param type le type du message global
 *
 * @return un nouvel élément du tableau de gros messages
 */
bm_node* new_bm_node(unsigned short total_size,unsigned char id[8], unsigned char nonce[4],unsigned char type) {
  bm_node* c_node = malloc(sizeof(bm_node));
  if (c_node == NULL)
    return NULL;

  c_node->list = create_blist();
  c_node->total_size = total_size;
  memcpy(c_node->id, id, 8);
  memcpy(c_node->nonce, nonce, 4);
  c_node->type = type;
  c_node->displayed = 0;
  return c_node;
}


/**
 * @brief Récupère l'élément identifié par (id, nonce) dans le tableau de gros messages
 *
 * @param c_list le tableau de gros messages
 * @param id l'id du message global
 * @param nonce la nonce  du message global
 *
 * @return le noeud identifié par (id,nonce)
 */
bm_node* get_from_bma(bm_array* c_list,unsigned char id[8], unsigned char nonce[4]) {
  for(int i=0; i < c_list->size; i++) {
    bm_node* elem = c_list->circ_list[i];

    if (elem != NULL) {
      short same_id = memcmp(id, elem->id, 8) == 0;
      short same_nonce = memcmp(nonce, elem->nonce, 4) == 0;

      if (same_id && same_nonce)
	return elem;
    }
  }

  return NULL;
}

/**
 * @brief Ajoute un élément au tableau de gros message (cad un TLV Data de type 220)
 *
 * @param msg le sous-message à ajouter
 * @param id l'id du message global
 * @param nonce la nonce du message global
 * @param type le type du message global
 * @param total la taille total du message global
 * @param number place du premier octet du sous-message dans le message global
 * @param len la taille du sous-message (de 0 à 233)
 *
 * @return void
 */
void add_to_bma(bm_array* c_list,unsigned char msg[233], unsigned char id[8], unsigned char nonce[4], unsigned char type,unsigned short total,
		        unsigned short number,unsigned char len) {
  bm_node* msg_node = get_from_bma(c_list, id, nonce);
  if (msg_node == NULL) {

    msg_node = new_bm_node(total, id, nonce, type);

    if (c_list->circ_list[c_list->index] !=  NULL) {
      free_blist(c_list->circ_list[c_list->index]->list);
      free(c_list->circ_list[c_list->index]);
    }
    c_list->circ_list[c_list->index] = msg_node;
    c_list->index = (c_list->index + 1) % c_list->size;
  }

  add_to_blist(msg_node->list, number, (char*)msg, len);
}

/**
 * @brief Récupère le message global identifié par (id, nonce) ssi il est complet
 *        et qu'il n'a pas encore été affiché (NULL Sinon)
 *
 * @param id  l'id du message global
 * @param nonce la nonce du message global
 *
 * @return le message global identifié par (id,nonce)
 */
char* get_bma_msg(bm_array* clist,unsigned char id[8],unsigned char nonce[4]) {
  bm_node* msg_node = get_from_bma(clist, id, nonce);
  if (msg_node == NULL || msg_node->total_size != msg_node->list->current_size ||
      msg_node->displayed) {
    return NULL;
  }

  return concat_list(msg_node->list, msg_node->total_size);
}

/**
 * @brief Indique à la struture que le message identifié par (id,nonce) à déjà été affiché
 *
 * @param clist le tableau de gros message
 * @param id l'id du message global
 * @param nonce la nonce du message global
 *
 * @return void
 */
void update_display(bm_array* clist,unsigned char id[8],unsigned char nonce[4]) {
  bm_node* msg_node = get_from_bma(clist, id, nonce);
  if (msg_node != NULL) {
    msg_node->displayed = 1;
  }
}

// FONCTION D'AFFICHAGE //
void pr_arr(bm_array* clist) {
  for (int i=0; i < clist->size; i++) {
    bm_node* node = clist->circ_list[i];
    if (node == NULL)
      continue;
    printf("NODE ----- %d -------\n",i);
    printf("%d total size\n",node->total_size);
    pr_blist(node->list);
    printf("____________________\n");
  }

}
