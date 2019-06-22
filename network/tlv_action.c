#include "headers/tlv_action.h"
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include "headers/transmitter.h"

#include "../interface/headers/chat_win_ctrl.h"
#include "../library/headers/utilities.h"

#define true 1
#define false 0

/**
 * @brief Fonction permettant de réaliser l'actcharon d'un TLV Hello
 *
 * @param pair courant
 * @param tampon la requête
 * @param sndr_ip ip de l'émetteur de la requête
 * @param sndr_port port de l'émetteur de la requête
 * @return 1 si tout se passe bien, 0 sinon
 */
short hello_action(pair* p,unsigned char* tampon, unsigned char sndr_ip[16], unsigned short sndr_port){
  char tlv_len = tampon[1]; //TAILLE DU TLV
  unsigned char sndr_id[8],rcver_id[8]; //A EXTRAIRE DU TLV
  memcpy(sndr_id, tampon+2, 8); //INITIALISATION

  if (tlv_len == 16) {
    memcpy(rcver_id,tampon+10, 8); //INITIALISATION

    //VERIFIE QUE L'ID INDIQUER DANS LE TLV EST BIEN LE NOTRE
    short same_id = memcmp(rcver_id, p->id, 8) == 0;
    if (same_id == true) {
      addNeighb(p, sndr_id, sndr_ip, sndr_port);
      pthread_mutex_lock(&p->mutexs.neighb_mutex);
      update_last_hello_rcv(p->neighbours, sndr_ip, sndr_port);
      pthread_mutex_unlock(&p->mutexs.neighb_mutex);
      return true;
    } else {
      sendTLVGoAway(p, 3, "NOT SAME ID AS HELLO LONG", sndr_ip, sndr_port);
    }

  } else if (tlv_len == 8) {
    addNeighb(p, sndr_id, sndr_ip, sndr_port);
    return true;
  }

  return false;
}

/**
 * @brief Fonction permettant de réaliser l'action d'un TLV Neighbour
 *
 * @param pair courant
 * @param tampon la requête
 * @param sndr_ip ip de l'émetteur de la requête
 * @param sndr_port port de l'émetteur de la requête
 * @return 1 si tout se passe bien, 0 sinon
 */
short neighbour_action(pair* p,unsigned char* tampon,unsigned char sndr_ip[16], unsigned short sndr_port) {
  if (!is_in_hashmap(p->neighbours, sndr_ip, sndr_port)){
    sendTLVGoAway(p, 3, "YOUR NOT MY NEIGHB", sndr_ip, sndr_port);
    return false;
  }

  unsigned char ip[16]; unsigned short port; //PARAMETRE A EXTRAIRE
  memcpy(ip, tampon+2, 16); //INITIALISATION
  memcpy(&port, tampon + 18, 2); //INITIALISATION
  port = ntohs(port);
  
  if (!is_in_hashmap(p->neighbours, ip, port)){
    addPotNeighb(p, ip, port); //ON AJOUTE QUE SI ON NE L'AS PAS EN VOISIN
  }

  return true;
}

/**
 * @brief Fonction permettant de réaliser l'action d'un TLV Data
 *
 * @param pair courant
 * @param tampon la requête
 * @param sndr_ip ip de l'émetteur de la requête
 * @param sndr_port port de l'émetteur de la requête
 * @return 1 si tout se passe bien, 0 sinon
 */
short data_action(pair* p,unsigned char* tampon, unsigned char sndr_ip[16],unsigned short sndr_port) {
  if (!is_in_hashmap(p->neighbours, sndr_ip, sndr_port)) {
    sendTLVGoAway(p, 3, "YOU'RE NOT MY NEIGHBOUR !", sndr_ip, sndr_port);
    return false;
  }

  unsigned char id[8],nonce[4],type,tlv_length,data[242];

  tlv_length = (unsigned char)tampon[1];

  memcpy(id, tampon + 2, 8);
  memcpy(nonce, tampon + 10, 4);
  type = tampon[14];
  memcpy(data, tampon + 15, tlv_length - 13);

  if (type == 0)
    {
      if(!exist(p -> inondation_list,id,nonce)){
	display_msg_shell(tampon+15,tlv_length - 13);
	add_msg_to_box((char*)data, tlv_length - 13);
      }
    }
  else if (type == 220)
    {
      unsigned char nonce2[4];
      memcpy(nonce2, data, 4);
      add_bma(p, data, id, tlv_length);
      char* msg = get_bma_msg(p->bm_arr, id, nonce2);
      if (msg != NULL) {
	int len = strlen(msg);
	display_msg_shell((unsigned char *)msg, len);
	add_msg_to_box(msg, len);
	update_bmdisplay(p, id, nonce2);	
      }
      
    }

  addMessageToIndList(p, id, nonce, data,type,tlv_length - 13);
  sendTLVAck(p, id, nonce, sndr_ip, sndr_port);

  return 1;
}

/**
 * @brief Fonction permettant de réaliser l'action d'un TLV Ack
 *
 * @param pair courant
 * @param tampon la requête
 * @param sndr_ip ip de l'émetteur de la requête
 * @param sndr_port port de l'émetteur de la requête
 * @return 1 si tout se passe bien, 0 sinon
 */
short ack_action(pair* p,unsigned char* tampon, unsigned char sndr_ip[16],unsigned short sndr_port){
  if (!is_in_hashmap(p->neighbours, sndr_ip, sndr_port)) {
    sendTLVGoAway(p, 3, "YOUR NOT MY NEIGHB", sndr_ip, sndr_port);
    return false;
  }

  unsigned char id[8],nonce[4];
  memcpy(id, tampon + 2, 8);
  memcpy(nonce, tampon + 10, 4);

  m_list* msg = get_from_clist(p->inondation_list, id, nonce);

  if (msg != NULL) {
    pthread_mutex_lock(&p->mutexs.circ_l_mutex);
    remove_from_hashmap(msg->neighb, sndr_ip, sndr_port);
    pthread_mutex_unlock(&p->mutexs.circ_l_mutex);
  }

  return true;
}

/**
 * @brief Fonction permettant de réaliser l'action d'un TLV GoAway
 *
 * @param pair courant
 * @param tampon la requête
 * @param sndr_ip ip de l'émetteur de la requête
 * @param sndr_port port de l'émetteur de la requête
 * @return 1 si tout se passe bien, 0 sinon
 */
short goAway_action(pair* p,unsigned char* tampon, unsigned char sndr_ip[16], unsigned short sndr_port){
  if (!is_in_hashmap(p->neighbours, sndr_ip, sndr_port)) {
    sendTLVGoAway(p, 3, "YOUR NOT MY NEIGHB", sndr_ip, sndr_port);
    return false;
  }

  unsigned short code = tampon[2];

  if (code != 1) {
    addPotNeighb(p, sndr_ip, sndr_port);
  }

  removeNeighb(p, sndr_ip, sndr_port);
  return true;
}

/**
 * @brief Fonction permettant de réaliser l'action d'un TLV Warning
 *
 * @param pair courant
 * @param tampon la requête
 * @return 1 si tout se passe bien, 0 sinon
 */
void warning_action(unsigned char* tampon) {
  printf("WARNING : %.*s \n",tampon[1],tampon + 2);
}
