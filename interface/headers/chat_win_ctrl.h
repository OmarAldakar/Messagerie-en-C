#ifndef CWIN_H
#define CWIN_H

#include <gtk/gtk.h>
#include "../../network/headers/pair.h"

short display_chat_window(const char* pseudo, short port);

void add_msg_to_box (char* msg, int size);
  
#endif
