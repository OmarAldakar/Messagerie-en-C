#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SIZE 1000
#define true 1
#define false 0

typedef struct IP_Port IP_Port;
struct IP_Port
{
  unsigned char ip[16];
  unsigned short port;
};

/**
 * @brief Structure correspondant à un noeud de la hashmap
 *
 */
typedef struct hashnode hashnode;
struct hashnode
{
  unsigned char id[8];
  IP_Port ip_port; // key

  time_t first_hello_rcv;
  time_t last_hello_rcv;

  time_t last_hello_sent;
  time_t last_neighb_sent;

  time_t last_data_sent;
  int nbr_transmit;
  hashnode* next; // utile pour la méthode de chaînage
};

/**
 * @brief Structure correspondant à une hashmap
 *
 */
typedef struct hashmap hashmap;
struct hashmap{
  int size;
  hashnode **list;
};

hashmap* create_hashmap(int size);

int hashcode(hashmap *tab,unsigned char ip[16],unsigned short port);

short insert_hashmap(hashmap *tab, unsigned char id[8], unsigned char ip[16], unsigned short port);

hashnode* search_hashmap(hashmap* tab, unsigned char ip[16],unsigned short port);

void clear_hashmap(hashmap *tab);

void delete_hashmap(hashmap *tab);

hashmap* copy_hashmap(hashmap* hashmap);

short remove_from_hashmap(hashmap* tab, unsigned char ip[16],unsigned short port);

void pr_hashmap(hashmap* hashmap);

short is_in_hashmap(hashmap* hashmap,unsigned char ip[16],short port);

void update_last_hello_rcv(hashmap* hashmap,unsigned char ip[16],short port);

#endif
