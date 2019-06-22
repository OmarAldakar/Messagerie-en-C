#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "headers/tlv_maker.h"
#include "../library/headers/utilities.h"


/**
 * @brief Fonction permettant de créer un TLV "short" Hello
 *
 * @param sender id de l'émetteur
 * @return char* le TLV correspondant à la description ci-dessus
 */
char* tlv_HelloShort(unsigned char sender[8]) {
  //LA TAILLE DU TLV HELLO SHORT EST TOUJOURS EGALE A 10
  char* tlv_hello_short = malloc(10);
  tlv_hello_short[0] = (unsigned char)2; // -> type du TLV
  tlv_hello_short[1] = (unsigned char)8; // -> taille du TLV
  memcpy(&tlv_hello_short[2], sender, 8); // -> id du transmetteur

  return tlv_hello_short;
}

/**
 * @brief Fonction permettant de créer un TLV "long" Hello
 *
 * @param sender id de l'émetteur
 * @param recv id du destinataire
 * @return char* le TLV correspondant à la description ci-dessus
 */
char* tlv_HelloLong(unsigned char sender[8], unsigned char recv[8]){
  //LA TAILLE DU TLV HELLO SHORT EST TOUJOURS EGALE A 18
  char* tlv_hello_long = malloc(18);
  tlv_hello_long[0] = (unsigned char)2; // -> type du TLV
  tlv_hello_long[1] = (unsigned char)16; // -> taille du TLV
  memcpy(&tlv_hello_long[2], sender, 8); // -> id du transmetteur
  memcpy(&tlv_hello_long[10], recv, 8); // -> id du receveur

  return tlv_hello_long;
}

/**
 * @brief Fonction permettant de créer un TLV Neighbour
 *
 * @param ip
 * @param port
 * @return char* le TLV correspondant à la description ci-dessus
 */
char* tlv_Neighbour(unsigned char ip[16], unsigned short port){
  //LA TAILLE DU TLV NEIGHBOURS EST TOUJOURS EGALE A 19
  int size = 18; // taille de l'ip + le port 2 OCTET !!
  char* tlv_ngb = malloc(20);
  tlv_ngb[0] = (unsigned char)3; // -> type du TLV
  tlv_ngb[1] = (unsigned char)size; // -> taille du TLV
  memcpy(tlv_ngb + 2, ip, 16); // -> ip d'un voisin
  port = htons(port);
  memcpy(tlv_ngb + 18, &port, 2);

  return tlv_ngb;
}

/**
 * @brief Fonction permettant de créer un TLV Data
 *
 * @param sender id de l'émetteur
 * @param msg message que l'émetteur veut envoyer
 * @param msg_size taille du message
 * @param type type du TLV Data
 * @return char* le TLV correspondant à la description ci-dessus
 */
char* tlv_Data(unsigned char sender[8], char* msg, unsigned char msg_size, unsigned char type) {
  char* tlv_data = malloc(msg_size + 15);
  tlv_data[0] = (unsigned char) 4; // -> type du TLV
  tlv_data[1] = msg_size + 13; // -> taille du TLV
  memcpy(tlv_data + 2, sender, 8);
  gen_random((unsigned char *)tlv_data + 10, 4);
  tlv_data[14] = type;
  memcpy(tlv_data + 15, msg, msg_size);

  return tlv_data;
}

/**
 * @brief Fonction permettant de créer un TLV Data de type 220
 *
 * @param sender id de l'émetteur
 * @param total la taille total du message global
 * @param msg le sous-message à envoyer
 * @param num le numéro du premier octet du sous-message dans le message global
 * @param nonce du message global
 * @param msg_size la taille du sous-message
 *
 * @return char* le TLV correspondant à la description ci-dessus
 */
char* tlv_big_data(unsigned char sender[8], unsigned short total, char msg[233],
		   unsigned short num, unsigned char nonce[4],unsigned char msg_size) {
  total = htons(total);
  num = htons(num);

  char* tlv_data = malloc(msg_size + 15 + 9);
  tlv_data[0] = (unsigned char) 4;
  tlv_data[1] = msg_size + 13 + 9;
  memcpy(tlv_data + 2, sender, 8);
  gen_random((unsigned char *)tlv_data + 10, 4);
  tlv_data[14] = (unsigned char)220;
  
  memcpy(tlv_data + 15, nonce, 4);
  tlv_data[19] =(unsigned char) 0; 
  memcpy(tlv_data + 20, &total, 2);
  memcpy(tlv_data + 22, &num, 2);
  memcpy(tlv_data + 24, msg, msg_size);
  return tlv_data;
}


/**
 * @brief Fonction permettant de créer un TLV Ack
 *
 * @param sender id de l'émetteur
 * @param nonce la nonce envoyé par l'émetteur et qui se trouve dans un TLV Data
 * @return char* le TLV correspondant à la description ci-dessus
 */
char* tlv_Ack(unsigned char sender[8], unsigned char nonce[4]){
    // UN TLV ACK AURA TOUJOURS POUR TAILLE 14
  char* tlv_ack = malloc(14);
  tlv_ack[0] = (unsigned char)5; // -> type du TLV
  tlv_ack[1] = (unsigned char)12; // -> taille du TLV
  memcpy(tlv_ack + 2, sender, 8); // -> id du transmetteur
  memcpy(tlv_ack + 10, nonce, 4);
  return tlv_ack;
}

/**
 * @brief Fonction permettant de créer un TLV GoAway
 *
 * @param code prenant comme valeur :
     0 - si la raison inconnue
     1 - si l’émetteur quitte le réseau
     2 - si le récepteur n’a pas envoyé un Hello depuis trop longtemps
         ou alors qu'il n’a pas acquitté un TLV Data dans les temps
     3 - si le récepteur a violé le protocole
     4 - si l’émetteur a déjà trop de voisins
 * @param msg un message optionnel pour le déboggage
 * @param length taille du message
 * @return char* le TLV correspondant à la description ci-dessus
 */
char* tlv_GoAway(int code, char* msg,unsigned char length){
    char* tlv_ga = malloc((length+3));
    tlv_ga[0] = (unsigned char)6;  // -> type du TLV
    tlv_ga[1] = (unsigned char)(length+1);  // -> taille du TLV
    tlv_ga[2] = (unsigned char)code;
    memcpy(tlv_ga + 3, msg, length);

    return tlv_ga;
}

/**
 * @brief Fonction permettant de créer un TLV Warning
 *
 * @param msg un message optionnel
 * @param length taille du message
 * @return char* le TLV correspondant à la description ci-dessus
 */
char* tlv_Warning(char* msg,unsigned char length){
    char* tlv_war = malloc((length+2));
    tlv_war[0] = (unsigned char)6;  // -> type du TLV
    tlv_war[1] = length;  // -> taille du TLV
    memcpy(tlv_war + 2, msg, length);

    return tlv_war;
}
