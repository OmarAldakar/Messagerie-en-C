#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <netdb.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <readline/readline.h>

#include "headers/utilities.h"

#define true 1
#define false 0

/**
 * @brief Genere un entier aléatoire de size octets
 * le stocke dans data
 *
 * @return void
 */
void gen_random(unsigned char* data,int size) {
  for (int i=0; i < size; i++) {
    data[i] = (rand() % 255);
  }
}

/**
 * @brief Traduit une chaine de caractère qui correspond à un host
 * en (ip,port) avec ip est une adresse IPV6
 *
 * @param adresse de l'host
 * @param port de l'host
 * @param endroit où on stocke le resultat
 * @param endroit où on stocke le resultat
 *
 * @return 1 si la fonction s'éxécute sans soucis 0 sinon
 */
short resolve_host(char* ip, char* port,unsigned char ip_stock[16], short* port_s) {
  struct addrinfo hints = {0};
  hints.ai_family   = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = 0;
  hints.ai_flags    = AI_V4MAPPED|AI_ALL;
  struct addrinfo* res = {0};

  int exit_status = getaddrinfo(ip,port, &hints, &res);

  if (exit_status != 0){
    perror("getaddrinfo:");
    return false;
  }

  struct sockaddr_in6* p_neighb = (struct sockaddr_in6*)res->ai_addr;
  memcpy(ip_stock, p_neighb->sin6_addr.s6_addr, 16);
  *port_s = ntohs(p_neighb->sin6_port);
  return true;
}

/**
* @brief renvoie le min entre 2 entiers
*
* @param a entier
* @param b entier
*
* @return le minimum entre les 2 entiers en paramètre
*/
int min(int a, int b) {
  return (a < b)?a:b;
}

// FONCTION D'AFFICHAGE //

void display_msg_shell(unsigned char* tampon, int tlv_length){
    printf("\r\033[K");
    printf("%.*s\n",tlv_length,tampon);
    printf("%.*s",rl_end,rl_line_buffer);
    fflush(stdout);
}
