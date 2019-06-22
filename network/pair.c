#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>


#include "headers/pair.h"
#include "../library/headers/utilities.h"

#include "headers/transmitter.h"
#include "time.h"

#define true 1
#define false 0
#define HASHMAP_SIZE 100
#define CIRC_LIST_SIZE 600
#define BIG_ARRAY_SIZE 100

#define CLEAN_POTN 90
#define CLEAN_N 45

/**
 * @brief Fonction permettant de créer un pair
 *
 * @param port
 * @pram pseudo
 *
 * @return pair* retourne un pointeur vers un pair
 */
pair* create_pair(unsigned short port, char* pseudo) {
  srand(time(NULL));
  pair* p = malloc(sizeof(pair));

  if (p == NULL)
    return NULL;

  gen_random(p->id, 8);
  p -> pseudo = strdup(pseudo);
  p -> p_neighb = new_list();
  p -> neighbours = create_hashmap(HASHMAP_SIZE);
  p -> inondation_list = init_clist(CIRC_LIST_SIZE);
  p -> bm_arr = init_bma(BIG_ARRAY_SIZE);

  pthread_mutex_init(&p->mutexs.circ_l_mutex, NULL);
  pthread_mutex_init(&p->mutexs.neighb_mutex, NULL);
  pthread_mutex_init(&p->mutexs.p_neighb_mutex, NULL);
  pthread_mutex_init(&p->mutexs.arr_mutex, NULL);

  short status = init_socket(p,port);
  if (status == true) {
    return p;
  }

  return NULL;
}

/**
 * @brief Fonction rajoutant à la liste de voisins du pair l'id-ip-port d'un voisin
 *
 * @param  p pair dont on veut mettre à jour la liste de voisins
 * @param id du voisin
 * @param ip du voisin
 * @param port du voisin
 * @return 1 si tout se passe bien, 0 sinon
 */
short addNeighb(pair *p,unsigned char id[8],unsigned char ip[16],short port) {
  pthread_mutex_lock(&p->mutexs.neighb_mutex);
  insert_hashmap(p->neighbours, id, ip, port);
  pthread_mutex_unlock(&p->mutexs.neighb_mutex);
  removePotNeighb(p, ip, port);
  return true;
}

/**
 * @brief Fonction rajoutant à la liste des potentiels voisins du pair l'ip-port
 *        (Cas où ce sont les voisins qui donne l'ip et le port)
 *
 * @param p pair dont on veut mettre à jour la liste de voisins potentiels
 * @param ip du voisin potentiel
 * @param port du voisin potentiel
 * @return 1 si tout se passe bien, 0 sinon
 */
short addPotNeighb(pair* p, unsigned char ip[16], short port) {
  pthread_mutex_lock(&p->mutexs.p_neighb_mutex);
  p->p_neighb = add(p->p_neighb, ip, port);
  pthread_mutex_unlock(&p->mutexs.p_neighb_mutex);
  return true;
}


/**
 * @brief Fonction rajoutant à la liste des potentiels voisins du pair l'ip-port
 *        (Cas où c'est l'utilisateur qui donne l'ip et le port)
 *
 * @param p pair dont on veut mettre à jour la liste de voisins potentiels
 * @param ip du voisin potentiel
 * @param port du voisin potentiel
 * @return 1 si tout se passe bien, 0 sinon
 */
short addPotNeighbString(pair* p, char* ip, char* port) {
  unsigned char ip_stock[16];
  short port_stock;

  //TRADUIT LE COUPLE (ip,port)
  short status = resolve_host(ip, port, ip_stock, &port_stock);
  if (status == true) {
    addPotNeighb(p, ip_stock, port_stock);
    return true;
  }

  return false;
}


/**
 * @brief Initialise la socket du pair passé en argument
 *
 * @param p pair dont on initialise la socket
 * @return 1 si tout se passe bien, 0 sinon
 */
short init_socket(pair * p,unsigned short port) {
  //INITIALISE LA SOCKET
  p->socket = socket(AF_INET6, SOCK_DGRAM, 0);
  if (p->socket == -1)
    return false;

  //LA MET EN MODE POLYMORPHE
  int monomorphe = 0;
  setsockopt(p->socket, IPPROTO_IPV6, IPV6_V6ONLY,&monomorphe,sizeof(monomorphe));

  //LA MET EN MODE NON-BLOQUANT
  int ex_stat = fcntl(p->socket, F_GETFL);
  if (ex_stat < 0)
    return false;

  ex_stat = fcntl(p->socket, F_SETFL, ex_stat| O_NONBLOCK);
  if (ex_stat < 0)
    return false;

  struct sockaddr_in6 server = {0};
  server.sin6_family = AF_INET6;
  server.sin6_port = htons(port);

  //FAIT ECOUTER LA SOCKET SUR LE PORT 1212
  return bind(p->socket, (struct sockaddr*)&server, sizeof(server)) == 0;
}


/**
 * @brief Fonction supprimant un utilisateur de la liste de voisins potentiels
 *
 * @param p pair dont on veut mettre à jour la liste de voisins
 * @param ip du voisin potentiel
 * @param port du voisin potentiel
 * @return 1 si tout se passe bien, 0 sinon
 */
short removePotNeighb(pair *p, unsigned char ip[16], short port) {
  pthread_mutex_lock(&p->mutexs.p_neighb_mutex);
  p->p_neighb = delete(p->p_neighb, ip, port);
  pthread_mutex_unlock(&p->mutexs.p_neighb_mutex);

  return true;
}

/**
 * @brief Fonction rajoutant un message à la liste d'inondation
 *
 * @param p pair dont on veut mettre à jour la liste d'inondation
 * @param id de l'utitilisateur qui nous envoie le message
 * @param nonce associé au paramètre précédent
 * @param message
 * @return 1 si tout se passe bien, 0 sinon
 */
short addMessageToIndList(pair* p, unsigned char id[8], unsigned char nonce[4],unsigned char msg[4096], unsigned char type, unsigned char length){
  pthread_mutex_lock(&p->mutexs.circ_l_mutex);
  add_msg(p->inondation_list, p->neighbours, msg, id, nonce,type,length);
  pthread_mutex_unlock(&p->mutexs.circ_l_mutex);
  return true;
}


/**
 * @brief Fonction supprimant un utilisateur de la liste de voisins
 *
 * @param p pair dont on veut mettrex à jour la liste de voisins
 * @param id du voisin
 * @return 1 si tout se passe bien, 0 sinon
 */
short removeNeighb(pair* p, unsigned char ip[16],short port) {
  pthread_mutex_lock(&p->mutexs.neighb_mutex);
  remove_from_hashmap(p->neighbours, ip, port);

  for (int i=0; i < p->inondation_list->size; i++) {
    m_list* elem = p->inondation_list->msg_l[i];
    if (elem != NULL) {
      remove_from_hashmap(elem->neighb, ip, port);
    }
  }
  pthread_mutex_unlock(&p->mutexs.neighb_mutex);
  return true;
}


/**
* @brief Fonction de mettre à jour la liste de voisins potentiels
*
* @param p pair dont on veut mettre à jour la liste de voisins potentiels
*
* @return void
*/
void cleanPotNeighb(pair* p){
    p_list* tmp = p -> p_neighb;
    while(tmp != NULL){
      time_t diff_time = time(NULL) - (tmp -> p_nei.first_contact);
      if(tmp->p_nei.first_contact != -1 && diff_time > CLEAN_POTN){
    	removePotNeighb(p,tmp->p_nei.ip, tmp->p_nei.port);
      }
      tmp = tmp -> next;
    }
}

/**
* @brief Fonction de mettre à jour la liste de voisins
*
* @param p pair dont on veut mettre à jour la liste de voisins
*
* @return void
*/
void cleanNeighb(pair* p){
  for(int i = 0; i < p -> neighbours -> size; i++){
    hashnode* ngb = p -> neighbours -> list[i];

    while(ngb != NULL){
      time_t diff_time = time(NULL) - ngb->last_hello_rcv;
      if(ngb->last_hello_rcv != -1 && diff_time > CLEAN_N) {
    	sendTLVGoAway(p, 2, "DIDN'T SEND HELLO FOR A LONG TIME !", ngb->ip_port.ip, ngb->ip_port.port);
    	pthread_mutex_lock(&p->mutexs.neighb_mutex);
    	remove_from_hashmap(p -> neighbours, ngb -> ip_port.ip , ngb -> ip_port.port);
    	pthread_mutex_unlock(&p->mutexs.neighb_mutex);
      }
      ngb = ngb->next;
    }
  }
}

/**
 * @brief Fonction parcourant la liste d'inondation et retire tous les voisin
 *        qui n'ont pas répondu après un certains nombre de message envoyé par
 *        le pair
 *
 * @param p pair dont on veut mettre à jour la liste d'inondation
 *
 * @return void
 */
void cleanData(pair* p){
    circ_list* tmp = p -> inondation_list;
    for(int i = 0; i < tmp -> size; i++){
        m_list* m_tmp = tmp -> msg_l[i];
        if(m_tmp != NULL){
            for(int j = 0 ; j < m_tmp -> neighb -> size; j++){
                hashnode* ngb = m_tmp -> neighb -> list[j];
	            while(ngb != NULL){
	                   if(ngb -> nbr_transmit >= 5){
                           removeNeighb(p, ngb->ip_port.ip, ngb->ip_port.port);
                           sendTLVGoAway(p, 3, "DID NOT SEND TLV ACK", ngb->ip_port.ip,
                           ngb->ip_port.port);
                        }
	            ngb = ngb -> next;
             }
          }
       }
    }
}

/**
 * @brief Ajoute un élément au tableau de gros message du pair (cad un TLV Data de type 220)
 *
 * @param p le pair
 * @param msg le TLV Data de type 220
 * @param id l'id du message global
 * @param len la taille du TLV Data
 *
 * @return void
 */
void add_bma(pair* p,unsigned char msg[242], unsigned char id[8],unsigned char len) {

  unsigned char data[233], nonce[4],type;
  unsigned short total;
  unsigned short number;

  memcpy(nonce, msg, 4);
  memcpy(&total, msg + 5, 2);
  memcpy(&number, msg + 7, 2);
  memcpy(data, msg + 9, len - 13 - 9);
  total = ntohs(total);
  number = ntohs(number);
  type = msg[4];

  pthread_mutex_lock(&p->mutexs.arr_mutex);
  add_to_bma(p->bm_arr, data, id, nonce, type, total, number, len - 13 - 9);
  pthread_mutex_unlock(&p->mutexs.arr_mutex);
}


/**
 * @brief Indique à la struture que le GROS message identifié par (id,nonce)
          à déjà été affiché
 *
 * @param p
 * @param id l'id du message global
 * @param nonce la nonce du message global
 *
 * @return void
 */
void update_bmdisplay(pair* p,unsigned char id[8],unsigned char nonce[4]) {
  pthread_mutex_lock(&p->mutexs.arr_mutex);
  update_display(p->bm_arr, id, nonce);
  pthread_mutex_unlock(&p->mutexs.arr_mutex);
}

// FONCTION D'AFFICHAGE //

void pr_pair(pair* p) {
  printf("POT NEIGHB LIST:\n");
  pr_list(p->p_neighb);
  printf("NEIGHB HASHMAP:\n");
  pr_hashmap(p->neighbours);
  printf("INONDATION LIST:\n");
  pr_circ_list(p->inondation_list);
}
