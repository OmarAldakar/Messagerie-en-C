#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "headers/pair.h"
#include "headers/receiver.h"
#include "headers/tlv_action.h"
#include "headers/transmitter.h"
#include "headers/send_opti.h"

/**
 * @brief Fonction permettant la réception d'un un TLV
 *
 * @param p notre pair
 * @return 1 si le TLV a bien été reçu, 0 sinon
 */
short receiveTLV(pair* p) {
  int exit_status;
  char tampon[SIZE];
  memset(tampon, 0, SIZE);

  struct sockaddr_in6 client;
  unsigned int client_length = sizeof(client);

  exit_status = recvfrom(p->socket, tampon, SIZE, 0,(struct sockaddr*) &client, &client_length);

  if (exit_status < 0) {
    perror("recv");
    return 0;
  }

  analyseTLV(p,(unsigned char*) tampon, client.sin6_addr.s6_addr, ntohs(client.sin6_port));
  return 1;
}

/**
 * @brief Fonction protégeant des "failles malicieuses" c'est-à-dire qui pourrait
 *        potentiellement donner des mauvaises tailles de TLV
 *
 * @param i position courante du TLV qu'on analyse
 * @param tampon la requête
 * @param taille la taille totale de la requête
 * @return short 1 si on dépasse pas la taille totale, 0 sinon
 */
short size_enough(int i, unsigned char* tampon,unsigned short taille) {
  return (i - 1 <= taille && tampon[i] == 0) ||
  (i - 1 <= taille && i-2 + tampon[i+1] <= taille);
}


/**
 * @brief Fonction permettant l'analyse d'un TLV
 *
 * @param p notre pair
 * @param tampon la requête
 * @param sder_ip ip de l'émetteur de la requête
 * @param s_port port de l'émetteur de la requête
 * @return void
 */
void analyseTLV(pair* p ,unsigned char* tampon, unsigned char sder_ip[16],unsigned short s_port) {
  if (tampon[0] != 93 || tampon[1] != 2){
    return;
  }

  unsigned short taille;
  memcpy(&taille, &tampon[2], 2);
  taille = ntohs(taille); //network order to byte order

  if (taille > 4093)
    return; //message non cohérant (trop grand)

  int i=4;
  while (size_enough(i, tampon, taille)) {
    switch(tampon[i]) {
    case 0:
      i+=1;
      break;
    case 1:
      i+= tampon[i + 1] + 2;
      break;
    case 2:
      hello_action(p,tampon + i,sder_ip,s_port);
      i+= tampon[i + 1] + 2;
      break;
    case 3:
      neighbour_action(p,tampon + i,sder_ip,s_port);
      i+= tampon[i + 1] + 2;
      break;
    case 4:
      data_action(p,tampon + i,sder_ip,s_port);
      i+= tampon[i + 1] + 2;
      break;
    case 5:
      ack_action(p,tampon + i,sder_ip,s_port);
      i+= tampon[i + 1] + 2;
      break;
    case 6:
      goAway_action(p,tampon + i,sder_ip,s_port);
      i+= tampon[i + 1] + 2;
      break;
    case 7:
      warning_action(tampon + i);
      i+= tampon[i + 1] + 2;
      break;
    }
  }

  trameSend(p,0);
}
