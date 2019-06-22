#ifndef DIS_H
#define DIS_H
#include <pthread.h>

pthread_t display_t;

void* display_thread(void* param);

#endif
