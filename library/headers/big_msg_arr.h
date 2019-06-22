#ifndef BMA_H
#define BMA_H

#include "big_msg_list.h"

/**
 * @brief Décrit un élément du tableau de gros messages
 *
 */
typedef struct bm_node {
  msg_list* list;
  unsigned short total_size;
  unsigned char id[8];
  unsigned char nonce[4];
  unsigned char type;
  short displayed;
} bm_node;

/**
 * @brief Le tableau pour stocker les TLV Data type 220 (gros messages)
 *
 */
typedef struct bm_array {
  bm_node** circ_list;
  int size;
  int index;
} bm_array;


bm_array* init_bma(int size);

void add_to_bma(bm_array* c_list,unsigned char msg[233], unsigned char id[8],unsigned char nonce[4], unsigned char type,unsigned short total,
		        unsigned short number,unsigned char len);

char* get_bma_msg(bm_array* clist,unsigned char id[8],unsigned char nonce[4]);

void update_display(bm_array* clist,unsigned char id[8],unsigned char nonce[4]);

void pr_arr(bm_array* clist);

#endif
