#include <stdio.h>
#include <unistd.h>
#include "headers/sender_th.h"
#include "../network/headers/transmitter.h"
#include "../network/headers/receiver.h"
#include "../network/headers/send_opti.h"

#define true 1
#define false 0

/**
 * @brief Fonction permettant l'envoie des requêtes récurrentes
 *
 * @param param pair courant
 * @return void
 */

void* sender_thread(void* param) {
  pair* p = (pair *) param;

  while(true){
    cleanPotNeighb(p);
    cleanNeighb(p);
    cleanData(p);
    sendTLVShortHello(p);
    trameSend(p,1);
    sleep(2);
  }

  return NULL;
}
