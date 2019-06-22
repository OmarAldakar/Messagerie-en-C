#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <readline/readline.h>

#include "headers/p_list.h"
#include "headers/utilities.h"

#define true 1
#define false 0

/**
 * @brief Ajoute un voisins potentiels au début de la liste
 *
 * @param list la liste de voisins potentiels
 * @param ip du voisin potentiel
 * @param port du voisin potentiel
 * @return p_list* la liste de voisin potentiel
 */
p_list* alloue(p_list* list, unsigned char ip[16],unsigned short port){
  p_list* list_aux = malloc(sizeof(p_list));
  memcpy(list_aux->p_nei.ip, ip, 16);
  list_aux->p_nei.port = port;
  list_aux->p_nei.first_contact = time(NULL);
  list_aux->p_nei.last_contact = -1;
  list_aux->next = list;

  return list_aux;
}

/**
 * @brief Initialise une liste de voisins potentiels vide
 *
 * @return p_list* une liste vide
 */
p_list* new_list() {
  return NULL;
}

short equal(p_neighb p_nei, unsigned char ip[16], unsigned short port) {
  return p_nei.port == port && (memcmp(p_nei.ip, ip, 16) == 0);
}


/**
 * @brief Regarde si l'élément d'adresse (ip,port) est dans la liste.
 *
 * @param list la liste des voisins potentiels
 * @param ip
 * @param port
 *
 * @return 1 l'élément est dans la liste 0 sinon
 */
short isInList(p_list* list, unsigned char ip[16],unsigned short port) {
  if (list == NULL)
    return false;

  if (equal(list->p_nei, ip,port))
    return true;

  return isInList(list->next, ip, port);
}

/**
 * @brief Ajoute un élément à la liste
 *
 * @param list La liste
 * @param ip L'addresse du voisin à ajouter
 * @param port Le port du voisin à ajouter
 *
 * @return La liste avec l'élément ajouté
 */
p_list* add(p_list* list, unsigned char ip[16],unsigned short port) {
    if (!isInList(list, ip, port)){
      return alloue(list, ip, port);
    }

  return list;
}


/**
 * @brief Enlève tous les élément d'adresse (ip,port) de la liste de voisins potentiels
 *
 * @param list liste des voisins potentiels
 * @param ip
 * @param port
 *
 * @return La liste sans les éléments
 */
p_list* delete(p_list* list,unsigned char ip[16],unsigned short port) {
  if (list == NULL)
    return list;

  if (equal(list->p_nei, ip, port)) {
    p_list* tmp = list->next;
    free(list);
    tmp = delete(tmp, ip, port);
    return tmp;
  } else {
    list->next = delete(list->next, ip, port);
    return list;
  }
}


// FONCTION D'AFFICHAGE //

void pr_list(p_list* list) {
  while(list != NULL) {
    char ip_str[INET6_ADDRSTRLEN];
    printf("ip : %s ",inet_ntop(AF_INET6, list->p_nei.ip,
				ip_str,INET6_ADDRSTRLEN));
    printf("port : %d \n",list->p_nei.port);

    list = list->next;
  }
  printf("\n");
}
