#include <stdlib.h>
#include <readline/readline.h>

#include "headers/display_th.h"
#include "../interface/headers/chat_win_ctrl.h"
#include "../network/headers/transmitter.h"
#define true 1
#define false 0

/**
 * @brief Fonction permettant de lire l'entrée courante
 *        et permettant les envois sur le terminal
 *
 * @param param pair courant
 * @return void*
 */
void* display_thread(void* param) {
  pair* p = (pair *) param;

  while (true) {
    char* line = readline("");
    printf("\033[1A\033[K");
    int name_size = strlen(p->pseudo);
    int data_size = strlen(line);
    char data [name_size + data_size + 3];
    data[0] = '\0';
    strcat(data, p->pseudo);
    strcat(data, ": ");
    strcat(data, line);
    printf("%s\n", data);
    add_msg_to_box(data, name_size + data_size + 3);
    sendTLVData(p, data);
    fflush(stdout);
    free(line);
  }

  return NULL;
}
