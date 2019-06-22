#ifndef THS_H
#define THS_H

#include <pthread.h>

pthread_t sender_t;

void* sender_thread(void* param);

#endif
