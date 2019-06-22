#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <signal.h>
#include <string.h>

#include "network/headers/pair.h"
#include "library/headers/utilities.h"
#include "threads/headers/sender_th.h"
#include "threads/headers/receiver_th.h"
#include "threads/headers/display_th.h"

#include "interface/headers/start_win_ctrl.h"

#define true 1
#define false 0


int main(int size,char** args) {
    //INITIALISE LE PAIR COURANT
    if(size <= 1){
        gtk_init(&size, &args);
	display_start_window();
        gtk_main();
    }
    else{
        unsigned short port = (unsigned short)atoi(args[1]);
        pair* p;
        if(size > 2){
            p = create_pair(port, args[2]);
        }
        else{
            p = create_pair(port,"Snow");
        }
        if (p == NULL) {
            perror("create_pair:");
            return EXIT_FAILURE;
        }

        //AJOUTE JCH.IRIF.FR AUX POTENTIELS VOISINS
        short status = addPotNeighbString(p, "localhost", "1213");
        if (status == false) {
            perror("addPotneighbstring");
            return EXIT_FAILURE;
        }
        //SI TOUS CE PASSE BIEN LANCE LES THREADS D'EXECUTION

        pthread_create(&reception_t, NULL, receiver_thread,p);
        pthread_create(&sender_t, NULL, sender_thread, p);
        pthread_create(&display_t, NULL,display_thread, p);

        pthread_join(sender_t, NULL);
        pthread_join(reception_t, NULL);
    }

    return EXIT_SUCCESS;
}
