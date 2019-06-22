#ifndef BML_H
#define BML_H

/**
 * @brief Décrit un noeud de la liste
 *
 */
typedef struct msg_node {
  unsigned short num;
  char msg[233];
  unsigned char len;
  struct msg_node* next;
} msg_node;


/**
 * @brief Décrit la liste chainée de sous-messages (la liste est ordonée)
 *
 */
typedef struct msg_list {
  msg_node* head;
  unsigned short current_size;
} msg_list;


void add_to_blist(msg_list* list,unsigned short num, char msg[233],unsigned char len);

void pr_blist(msg_list* list);

msg_list* create_blist();

void free_blist(msg_list* list);

char* concat_list(msg_list* list,unsigned short total_size);

#endif
