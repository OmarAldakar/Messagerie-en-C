#ifndef THR_H
#define THR_H
#include <pthread.h>

pthread_t reception_t;

void* receiver_thread(void* param);

#endif
