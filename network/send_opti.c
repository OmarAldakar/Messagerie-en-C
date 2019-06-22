#include <string.h>
#include <math.h>
#include "headers/pair.h"
#include "headers/transmitter.h"
#include "headers/tlv_maker.h"

/**
 * @brief Fonction permettant de concaténer un TLV Long Hello
 *		  dans la requête
 *
 * @param p pair courant
 * @param ngb le voisin destinataire
 * @param request la requête à remplir
 * @param pos position courante dans la requête
 * @param r_size taille effective de la requête
 * @return void
 */
void addTLVLongHello(pair* p, hashnode* ngb,unsigned char request[MTU], int* pos,unsigned short* r_size){
  time_t delta_time = time(NULL) - ngb->last_hello_sent;

  if (ngb->last_hello_sent != -1 && delta_time <= SHELLO_FREQ){
    return;
  }

  pthread_mutex_lock(&p->mutexs.neighb_mutex);
  ngb->last_hello_sent = time(NULL);
  pthread_mutex_unlock(&p->mutexs.neighb_mutex);

  char* tlv_hello_long = tlv_HelloLong(p->id,ngb->id);
  *r_size = *r_size + 18;
  concat(request, *pos, *r_size, tlv_hello_long, 18);
  *pos = *pos + 18;
  free(tlv_hello_long);
}

/**
 * @brief Fonction permettant de concaténer les TLV Neighbours
 *		  dans la requête
 *
 * @param p pair courant
 * @param ngb le voisin destinataire
 * @param request la requête à remplir
 * @param pos position courante dans la requête
 * @param r_size taille effective de la requête
 * @return void
 */
void addTLVNeighb(pair* p, hashnode* ngb,unsigned char request[MTU], int* pos,unsigned short* r_size) {
  time_t delta_time = time(NULL) - ngb->last_neighb_sent;
  if (ngb->last_neighb_sent != -1 && delta_time <= NEIGBH_FREQ){
    return;
  }

  pthread_mutex_lock(&p->mutexs.neighb_mutex);
  ngb->last_neighb_sent = time(NULL);
  pthread_mutex_unlock(&p->mutexs.neighb_mutex);

  for (int i=0; i < p->neighbours->size; i++) {
    hashnode* send_ngb = p->neighbours->list[i];
    while (send_ngb != NULL) {

      if ( 4 + *r_size + 20 > MTU) { //PLUS DE PLACE DONC NVX REQUEST
	send_request(p, ngb->ip_port.ip, ngb->ip_port.port, *r_size + 4, request);
	*pos = 4;
	init_header(request);
	*r_size = 0;
      }

      char* tlv_neighbour = tlv_Neighbour(send_ngb->ip_port.ip, send_ngb->ip_port.port);
      *r_size += 20;
      concat(request, *pos, *r_size, tlv_neighbour, 20);
      *pos = *pos + 20;
      free(tlv_neighbour);

      send_ngb = send_ngb->next;
    }
  }

}

/**
 * @brief Fonction renvoyant le temps d'attente en fonction
 *        du nombre de TLV Data transmis
 *
 * @param transmit_nbr nombre de TLV Data transmis
 * @return int le temps d'attente
 */
int get_time_to_wait(int transmit_nbr) {
  if (transmit_nbr < 1)
    return 0;

  int x1 = pow(2, transmit_nbr - 1);
  int x2 = pow(2, transmit_nbr);
  return  (x1 + x2) / 2;
}

/**
 * @brief Fonction permettant de concaténer les TLV Data
 *		  dans la requête
 *
 * @param p pair courant
 * @param ngb le voisin destinataire
 * @param request la requête à remplir
 * @param pos position courante dans la requête
 * @param r_size taille effective de la requête
 * @return void
 */
void addTLVData(pair* p, hashnode* ngb,unsigned char request[MTU], int* pos,unsigned short* r_size) {

  for (int i=0; i < p->inondation_list->size; i++) {
    m_list* elem = p->inondation_list->msg_l[i];
    if (elem == NULL) {
      continue;
    }
    hashnode* inond_ngb = search_hashmap(elem->neighb, ngb->ip_port.ip, ngb->ip_port.port);
    if (inond_ngb == NULL) {
      continue;
    }

    if ( 4 + *r_size + elem->length + 15 > MTU) { //PLUS DE PLACE DONC NVX REQUEST
      send_request(p, ngb->ip_port.ip, ngb->ip_port.port, *r_size + 4, request);
      *pos = 4;
      init_header(request);
      *r_size = 0;
    }

    time_t delta_time = time(NULL) - inond_ngb->last_data_sent;
    int time_to_wait = get_time_to_wait(inond_ngb->nbr_transmit);

    if (inond_ngb->last_data_sent == -1 || delta_time > time_to_wait) {
      pthread_mutex_lock(&p->mutexs.circ_l_mutex);
      inond_ngb->last_data_sent = time(NULL);
      inond_ngb->nbr_transmit = inond_ngb->nbr_transmit + 1;
      pthread_mutex_unlock(&p->mutexs.circ_l_mutex);

      unsigned char tlv_data[elem->length + 15];
      tlv_data[0] = 4; // -> type du TLV
      tlv_data[1] = elem->length + 13; // -> taille du TLV
      memcpy(tlv_data + 2, elem->id, 8);
      memcpy(tlv_data + 10, elem->nonce, 4);
      tlv_data[14] = elem->type;
      memcpy(tlv_data + 15, elem->msg, elem->length);

      *r_size = *r_size + 15 + elem->length;

      concat(request, *pos, *r_size,(char*)tlv_data, 15 + elem->length);
      *pos = *pos + 15 + elem->length;
    }
  }

}

/**
 * @brief Fonction permettant d'envoyer (tout en concaténant) les TLV :
 *          - LongHello
 *          - Neighbour
 *          - Data
 *        (Pour l'envoie du Data -> uniquement si le champ sendData vaut true)
 *
 * @param p pair courant
 * @param ngb le voisin destinataire
 * @param sendData
 * @return void
 */
void sendRequestToNeighb(pair*p, hashnode* ngb,short sendData) {
  //INIT_REQUEST
  unsigned char request[MTU];
  init_header(request);
  int pos = 4;
  unsigned short r_size = 0;
  addTLVLongHello(p, ngb, request, &pos, &r_size);
  addTLVNeighb(p, ngb, request, &pos, &r_size);

  if (sendData)
    addTLVData(p,ngb,request,&pos,&r_size);

  if (r_size != 0) {
    send_request(p, ngb->ip_port.ip, ngb->ip_port.port, r_size + 4, request);
  }
}

/**
 * @brief Fonction appellant sendRequestToNeighb pour tous les voisins du pair courant
 *
 * @param p pair courant
 * @param sendData
 * @return void
 */
void trameSend(pair* p,short sendData) {
  for(int i = 0; i < p->neighbours-> size; i++){
    hashnode* ngb = p->neighbours-> list[i];
    while(ngb != NULL){
      sendRequestToNeighb(p, ngb,sendData);
      ngb = ngb->next;
    }
  }
}
