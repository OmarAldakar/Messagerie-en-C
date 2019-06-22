#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/big_msg_list.h"

/**
 * @brief Crée un nouveau noeud
 *
 * @param num numéro du premier octet du sous-message dans le message global
 * @param msg le sous message en question
 * @param len la taille du sous-message
 *
 * @return un nouveau noeud
 */
msg_node* create_node(unsigned short num, char msg[233],unsigned char len) {
  msg_node* node = malloc(sizeof(msg_node));
  if (node == NULL)
    return NULL;

  node->num = num;
  node->len = len;
  memcpy(node->msg, msg, 233);
  return node;
}


/**
 * @brief Crée une liste vide
 *
 * @return la nouvelle liste
 */
msg_list* create_blist() {
  msg_list* list = malloc(sizeof(msg_list));
  if (list == NULL)
    return NULL;

  list->current_size = 0;
  list->head = NULL;
  return list;
}

/**
 * @brief Ajoute un nouveau noeud à la liste
 *
 * @param list la liste en question
 * @param num numéro du premier octet du sous-message dans le message global
 * @param msg le sous message en question
 * @param len la taille du sous-message
 *
 * @return un nouveau noeud
 */
void add_to_blist(msg_list* list,unsigned short num,char msg[233],unsigned char len) {
  msg_node* new_node = create_node(num, msg, len);
  if (new_node == NULL)
    return;

  msg_node* current_node = list->head;
  if (current_node == NULL || current_node->num > num) {
    list->head = new_node;
    new_node->next = current_node;
    list->current_size = list->current_size + len;
    return;
  }

  if (current_node->num == num) {
    return;
  }

  while (current_node->next != NULL) {
    if (current_node->next->num > num) {
      break;
    }
    if (current_node->next->num == num) {
      return;
    }

    current_node = current_node->next;
  }

  new_node->next = current_node->next;
  current_node->next = new_node;
  list->current_size = list->current_size + len;
}


/**
 * @brief Libère un noeud de la mémoire
 *
 * @param node le noeud en question
 *
 * @return void
 */
void free_bnode(msg_node* node) {
  if (node == NULL)
    return;
  free_bnode(node->next);
  free(node);
}

/**
 * @brief Libère une liste de la mémoire
 *
 * @param list la liste en question
 *
 * @return void
 */
void free_blist(msg_list* list) {
  if (list == NULL)
    return;
  free_bnode(list->head);
  free(list);
}


/**
 * @brief renvoie le gros message (la concaténation des sous-messages)
 *
 * @param list la liste de sous-messages
 *
 * @return le message en question
 */
char* concat_list(msg_list* list,unsigned short total) {
  char* str = malloc(total + 1);
  int pos = 0;
  msg_node* tmp = list->head;

  while (tmp != NULL) {
    memcpy(str + pos, tmp->msg, tmp->len);
    pos += tmp->len;
    tmp = tmp->next;
  }
  str[pos] = '\0';
  return str;
}


// FONCTION D'AFFICHAGE //
void pr_blist(msg_list* list) {
  printf("CURR_SIZE: %d\n", list->current_size);
  msg_node* curr_node = list->head;

  while(curr_node != NULL) {
    printf("MSG %d : %.*s \n",curr_node->num,curr_node->len,curr_node->msg);
    printf("MSG %d \n",curr_node->num);
    for (int i=0; i < curr_node->len; i++) {
      printf("%d ",(unsigned char)curr_node->msg[i]);
    }
    printf("\n");
    curr_node = curr_node->next;
  }
}
