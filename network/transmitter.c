#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <math.h>

#include "headers/transmitter.h"
#include "headers/tlv_maker.h"
#include "../library/headers/hashmap.h"
#include "../library/headers/utilities.h"
#include "headers/send_opti.h"

#define true 1
#define false 0

#define DATA_SIZE 242

/**
 * @brief Fonction remplissant l'entête d'une requête
 *
 * @param request la requête
 * @return void
 */
void init_header(unsigned char request[MTU]) {
  request[0] = 93;
  request[1] = 2;
}

/**
 * @brief Fonction permettant de rajouter les tlvs à la requête
 *
 * @param request requête à laquelle on rajoute les tlvs
 * @param i position à laquelle on rajoute le TLV
 * @param new_size nouvelle taille effective de la requête
 * @pamam tlv
 * @param tlv_len la taille du tlv qu'on ajoute
 * @return void
 */
void concat(unsigned char request[MTU],int i, short new_size, char* tlv,int tlv_len) {
  new_size = htons(new_size);
  memcpy(&request[2], &new_size, 2);
  memcpy(&request[i], tlv, tlv_len);
}

/**
 * @brief Fonction permettant d'envoyer une requête
 *
 * @param p pair émetteur
 * @param ip du destinataire
 * @param port du destinataire
 * @param r_size taille effective de la requête
 * @param request la requête à envoyer
 * @return short 1 si l'envoie s'est bien passé 0 sinon
 */
short send_request(pair* p, unsigned char ip[16], unsigned short port, int r_size, unsigned char request[MTU]){
  struct timeval tv = {1, 0};
  fd_set sendfds;
  FD_ZERO(&sendfds);
  FD_SET(p->socket, &sendfds);
  select(p->socket + 1, NULL, &sendfds, NULL, &tv);

  if(!FD_ISSET(p->socket, &sendfds)){
    return false;
  }

  struct sockaddr_in6 in6 = {0};
  in6.sin6_family = AF_INET6;
  in6.sin6_port = htons(port);
  memcpy(in6.sin6_addr.s6_addr, ip, 16);

  int exit_status = sendto(p->socket, request, r_size, 0, (struct sockaddr*)&in6,sizeof(in6));

  if (exit_status < 0) {
    if(errno == ENETUNREACH) {
      // ICI ON NE PEUT PAS LUI ENVOYER DE MESSAGE PARCE QU'ON EST PAS CONNECTER A IPV6
      return false;
    }
    perror("SENDTO");
  }
  return true;
}

/**
 * @brief Fonction ajoutant un message à la liste d'inondation du pair émetteur
 *
 * @param p pair dont on veut rajouter un message à sa liste d'inondation
 * @param tlv_data le tlv qui contient les informations nécessaire à l'ajout
 * @return void
 */
void addToInd(pair* p,char* tlv_data) {
  unsigned char tlv_len,data[242],id[8],nonce[4],type;
  tlv_len = (unsigned char)tlv_data[1];
  memcpy(id,tlv_data + 2, 8);
  memcpy(nonce,tlv_data + 10, 4);
  type = (unsigned char)tlv_data[14];
  memcpy(data,tlv_data + 15, tlv_len - 13);

  addMessageToIndList(p, id, nonce, data, type, tlv_len - 13);
}

/**
 * @brief Fonction transmettant TLV "short" Hello à sa liste de voisins potentiels (qui ne contiennent pas les ids !)
 *
 * @param pair
 * @return 1 si le TLV a bien été envoyé, 0 sinon
 */
short sendTLVShortHello(pair* p) {
  unsigned char request[MTU];
  init_header(request);

  //TAILLE DU TOTAL DU MESSAGE
  unsigned short size = 0;

  char* tlv_hello_short = tlv_HelloShort(p->id);
  size += 10;

  //CONCATENE LE TLV AU MESSAGE
  concat(request, 4, size, tlv_hello_short, 10);
  free(tlv_hello_short);

  for (p_list* l = p->p_neighb; l != NULL; l = l->next) {
    time_t delta_time = time(NULL) - l->p_nei.last_contact;
    if (l->p_nei.last_contact == -1 || delta_time > SHELLO_FREQ){
      pthread_mutex_lock(&p->mutexs.p_neighb_mutex);
      send_request(p, l->p_nei.ip, l->p_nei.port, (size+4),request);
      l->p_nei.last_contact = time(NULL);
      pthread_mutex_unlock(&p->mutexs.p_neighb_mutex);
    }
  }

  return true;
}

/**
 * @brief Fonction transmettant TLV Data de type 220
 *
 * @param pair
 * @param msg le message a envoyé
 * @param m_size la taille du message
 * @return 1 si le TLV a bien été envoyé, 0 sinon
 */
short sendTLVBigData(pair* p, char* msg,int m_size) {
  if (m_size > 65535) {
    return 0;
  }

  int cur_size = m_size;
  int compteur = 0;
  unsigned char nonce[4];
  gen_random(nonce, 4);

  while (cur_size > 0) {
    int data_len = min(233,cur_size);
    int pos = m_size - cur_size;

    char* tlv_data = tlv_big_data(p->id, m_size, msg + pos, compteur, nonce, data_len);
    compteur += data_len;

    addToInd(p, tlv_data);
    add_bma(p, (unsigned char *) tlv_data + 15 , p->id, tlv_data[1]);
    free(tlv_data);

    cur_size -= data_len;
  }

  update_bmdisplay(p, p->id, nonce);
  return 1;
}

/**
 * @brief Fonction transmettant TLV Data en émettant un message à la liste de voisins (fait également la fragmentation du message)
 *
 * @param pair
 * @param msg le message a envoyé
 * @return 1 si le TLV a bien été envoyé, 0 sinon
 */
short sendTLVData (pair* p, char* msg) {
  int m_size = strlen(msg);
  int cur_size = m_size;

  if (m_size > 242) {
    sendTLVBigData(p, msg, m_size);
    trameSend(p, 1);
    return true;
  }

  while (cur_size > 0) {
    int data_len = min(242,cur_size);
    int position = m_size - cur_size;

    char* tlv_data = tlv_Data(p->id, msg + position, data_len,0);
    addToInd(p, tlv_data);
    free(tlv_data);

    cur_size -= data_len;
  }

  trameSend(p, 1);
  return true;
}


/**
 * @brief Fonction transmettant un TLV Ack d'acquittement à partir du message reçu
 *
 * @param pair courant
 * @param recmsg le message reçu
 * @param ip du destinataire
 * @param port du destinataire
 * @return 1 si le TLV a bien été envoyé, 0 sinon
 */
short sendTLVAck(pair* p, unsigned char id[8], unsigned char nonce[4], unsigned char ip[16], short port){
  unsigned char request[MTU];
  init_header(request);

  char* tlv_ack = tlv_Ack(id,nonce);
  concat(request, 4, 14, tlv_ack, 14);
  free(tlv_ack);

  send_request(p, ip, port, 18,request);
  return true;
}

/**
 * @brief Fonction transmettant un TLV GoAway
 *
 * @param pair
 * @param code prenant comme valeur :
     0 - si la raison inconnue
     1 - si l’émetteur quitte le réseau
     2 - si le récepteur n’a pas envoyé un Hello depuis trop longtemps
         ou alors qu'il n’a pas acquitté un TLV Data dans les temps
     3 - si le récepteur a violé le protocole
     4 - si l’émetteur a déjà trop de voisins
 * @param msg un message optionnel pour le déboggage
 * @param ip du destinataire
 * @param port du destinataire
 * @return 1 si le TLV a bien été envoyé, 0 sinon
 */
short sendTLVGoAway(pair* p, int code, char* msg, unsigned char ip[16], short port) {
  int m_size = strlen(msg);
  if (m_size > 254) {
    return false;
  }
  //INITIALISATION DE LA REQUETE
  unsigned char request[MTU];
  init_header(request);
  int pos = 4;
  unsigned short r_size = 0;

  char* tlv_goaway = tlv_GoAway(code,msg,m_size);
  r_size += 3 + m_size;
  concat(request, pos, r_size, tlv_goaway, 3 + m_size);
  free(tlv_goaway);
  send_request(p, ip, port, r_size+4, request);

  return true;
}

/**
 * @brief Fonction transmettant un TLV Warning
 *
 * @param pair courant
 * @param msg un message optionnel
 * @param ip du destinataire
 * @param port du destinataire
 * @return 1 si le TLV a bien été envoyé, 0 sinon
 */
short sendTLVWarning(pair* p, char* msg, unsigned char ip[16], short port){
  int m_size = strlen(msg);
  if (m_size > 255) {
    return false;
  }
  //INITIALISATION DE LA REQUETE
  unsigned char request[MTU];
  init_header(request);
  int pos = 4;
  unsigned short r_size = 0;

  char* tlv_warning = tlv_Warning(msg,m_size);
  r_size += 2 + m_size;
  concat(request, pos, r_size, tlv_warning, 2 + m_size);
  pos += 3 + m_size;
  free(tlv_warning);

  send_request(p, ip, port, r_size+4, request);
  return true;
}
