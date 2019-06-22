#include <stdlib.h>
#include "headers/receiver_th.h"
#include "../network/headers/receiver.h"


/**
 * @brief Fonction permettant le traitement de la réception des requêtes
 *
 * @param param pair courant
 * @return void*
 */
void* receiver_thread(void* param) {
  pair* p = (pair*) param;

  while (true) {
    fd_set rcvfds;
    FD_ZERO(&rcvfds);
    FD_SET(p->socket, &rcvfds);
    select(p->socket + 1, &rcvfds, NULL, NULL, NULL);
    if(FD_ISSET(p->socket, &rcvfds))
      receiveTLV(p);
  }

  return NULL;
}
