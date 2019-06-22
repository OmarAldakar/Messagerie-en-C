#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include <readline/readline.h>
#include "headers/hashmap.h"

/**
 * @brief Crée une table de hashage
 *
 * @param size taille de la table de hashage
 * @return hashmap* un pointeur vers la table de hashage
 */
hashmap* create_hashmap(int size){
  hashmap* tab = malloc(sizeof(hashmap));
  tab -> size = size;
  tab -> list =  malloc(sizeof(hashnode*) * size);
  for(int i=0; i < size; i++) {
    tab -> list[i] = NULL;
  }
  return tab;
}

/**
 * @brief Génère un hashcode
 *
 * @param tab un pointeur vers la hashmap
 * @param id avec laquelle on veut générer le hashcode associé
 * @return int entier compris entre 0 et MAX_SIZE=1000
 */
int hashcode(hashmap *tab,unsigned char ip[16],unsigned short port){
  int hc = 0;
  for(int i=0; i < 16; i++){
    hc += ip[i];
  }
  hc += port;

  return hc % tab->size;
}

/**
 * @brief Permet de comparer 2 ids
 *
 * @param fst_id premier id qu'on veut comparer
 * @param snd_id second id qu'on veut comparer
 * @return renvoie 1 si les ids sont les mêmes, 0 sinon
 */
short equals_id(unsigned char fst_id[8],unsigned char snd_id[8]){
  return memcmp(fst_id, snd_id, 8) == 0;
}

/**
 * @brief Permet de comparer 2 ips/port
 *
 * @param ip1
 * @param ip2
 * @param port1
 * @param port2
 * @return renvoie 1 si les ips/port sont les mêmes, 0 sinon
 */
short equals_ip_port(unsigned char ip1[16],unsigned char ip2[16], short port1, short port2){
  return memcmp(ip1, ip2, 16) == 0 && port1 == port2;
}

/**
 * @brief Permet d'insérer un nouvel élément dans la table de hachage
 *
 * @param tab la table de hachage
 * @param id l'id de l'élément à insérer
 * @param ip_port l'ip et le port de l'élément à insérer
 * @return 1 si l'insertion a bien eu lieu 0 sinon
 */
short insert_hashmap(hashmap *tab, unsigned char id[8],unsigned char ip[16],
		     unsigned short port){
  int position = hashcode(tab,ip,port);
  hashnode *list = tab -> list[position];

  hashnode *tmp = list;
  while(tmp != NULL) { // On vérifie si l'id est déjà présent dans la hashmap
    if(equals_ip_port(tmp->ip_port.ip, ip, tmp->ip_port.port, port)){
      memcpy(tmp->id, id, 8);
      return true;
    }

    tmp = tmp -> next;
  }
  // Sinon on insère l'élément
  hashnode *newNode = malloc(sizeof(hashnode));

  memcpy(newNode->id, id, 8);
  memcpy(newNode->ip_port.ip, ip, 16);
  newNode->ip_port.port = port;

  newNode->last_data_sent = -1;
  newNode->last_hello_sent = -1;
  newNode->last_neighb_sent = -1;
  newNode->nbr_transmit = 0;

  newNode->first_hello_rcv = time(NULL);
  newNode->last_hello_rcv = time(NULL);

  newNode->next = list;
  tab->list[position] = newNode;
  return true;
}


/**
 * @brief cherche l'IP et le port associé à l'id donné en paramètre dans la table de hachage et la renvoie
 *
 * @param tab la table de hachage
 * @param id l'id qui va nous permettre de trouver l'ip et le port associé à l'id
 * @return IP_Port l'ip et le port qu'on cherche
 */
hashnode* search_hashmap(hashmap* tab, unsigned char ip[16],unsigned short port){
  int position = hashcode(tab,ip,port);
  hashnode *list = tab -> list[position];
  hashnode *tmp = list;
  while(tmp != NULL){
    if(equals_ip_port(ip, list->ip_port.ip, port, list->ip_port.port)){
      return tmp;
    }
    tmp = tmp -> next;
  }
  return NULL;
}

/**
 * @brief Libère la mémoire prise par une hashnode
 *
 * @param hashnode la node à libérer
 */
void free_hashnode(hashnode* node){
  free(node);
}

/**
 * @brief Enleve l'element id de la hashmap
 *
 * @param tab la table de hachage
 * @param id l'id a remove
 * @return void
 */
short remove_from_hashmap(hashmap* tab,unsigned char ip[16],unsigned short port){
  int position = hashcode(tab,ip,port);
  hashnode *list = tab -> list[position];

  hashnode *actual = list;
  hashnode *previous = list;

  if (actual != NULL &&  equals_ip_port(ip,actual->ip_port.ip ,
					port, actual->ip_port.port)) {
    tab->list[position] =  actual->next;
    free_hashnode(actual);
    return true;
  }

  while(actual != NULL){
    if(equals_ip_port(ip,actual->ip_port.ip,port, actual->ip_port.port)){
      previous->next = actual->next;
      free_hashnode(actual);
      return true;
    }
    previous = actual;
    actual = actual -> next;
  }

  return false;
}

/**
 * @brief vide la table de hachage et libère la mémoire
 *
 * @param tab pointeur vers la table de hachage
 * @return void
 */
void clear_hashmap(hashmap *tab){
  for(int i = 0; i < tab -> size; i++){
    hashnode *node = tab -> list[i];
    while(node != NULL){
      hashnode *tmp = node;
      node = node -> next;
      free_hashnode(tmp);
    }
    tab -> list[i] = NULL;
  }
}

/**
 * @brief supprime la table de hachage et libère la mémoire
 *
 * @param tab un pointeur vers la table de hachage
 * @return void
 */
void delete_hashmap(hashmap *tab){
  if(tab != NULL){
    clear_hashmap(tab);
    free(tab->list);
    free(tab);
  }
}


/**
 * @brief Permet de renvoyer une copie de la hashnode passé en paramètre
 *
 * @param node
 * @return hashnode* une copie du node passé en paramètre
 */
hashnode* copy_node(hashnode* node) {
  if (node == NULL) {
    return NULL;
  }

  hashnode* node_cpy = malloc(sizeof(hashnode));

  if (node_cpy == NULL) {
    return NULL;
  }

  memcpy(node_cpy->ip_port.ip, node->ip_port.ip, 16);
  memcpy(node_cpy->id, node->id, 8);
  node_cpy->ip_port.port = node->ip_port.port;
  node_cpy->last_data_sent = -1;
  node_cpy->nbr_transmit = 0;
  node_cpy->next = copy_node(node->next);
  return node_cpy;
}

/**
 * @brief Permet de renvoyer une copie de la hashmap passée en paramètre
 *
 * @param neighb
 * @return hashmap* une copie de la hashmap passé en paramètre
 */
hashmap* copy_hashmap(hashmap* neighb) {
  if (neighb == NULL) {
    return NULL;
  }

  hashmap* h_copy = malloc(sizeof(hashmap));

  if (h_copy == NULL)
    return NULL;

  h_copy->size = neighb->size;
  h_copy->list = malloc(sizeof(hashnode*) * h_copy->size);

  if (h_copy->list == NULL) {
    free(h_copy);
    return NULL;
  }

  for (int i=0; i < h_copy->size; i++) {
    h_copy->list[i] = copy_node(neighb->list[i]);
  }

  return h_copy;
}

// FONCTION D'AFFICHAGE //

void pr_hashnode(hashnode* h_n,int i) {
  if (h_n == NULL)
    return;

  printf("----------------- NODE %d: ---------------- \n",i);
  char ip_str[INET6_ADDRSTRLEN];
  printf("ip : %s \n",inet_ntop(AF_INET6, h_n->ip_port.ip,
				ip_str,INET6_ADDRSTRLEN));
  printf("port : %d \n",h_n->ip_port.port);
  printf("id : ");
  for(int i=0; i < 8; i++) {
    printf("%d ",h_n->id[i]);
  }
  printf("\n");
  pr_hashnode(h_n->next,i+1);
}

void pr_hashmap(hashmap* hashmap) {
  for (int i=0; i < hashmap->size; i++) {
    hashnode* h_n = hashmap->list[i];

    if (h_n != NULL) {
      printf("-----------------CASE %d: ---------------- \n",i);
      pr_hashnode(h_n, 1);
    }
  }

  printf("\n");
}


short is_in_hashmap(hashmap* hashmap,unsigned char ip[16],short port) {
  return search_hashmap(hashmap, ip, port) != NULL;
}

void update_last_hello_rcv(hashmap* hashmap,unsigned char ip[16],short port) {
  hashnode* h_n = search_hashmap(hashmap, ip, port);
  h_n->last_hello_rcv = time(NULL);
}
