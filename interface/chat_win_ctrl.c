#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "headers/chat_win_ctrl.h"
#include "../threads/headers/sender_th.h"
#include "../threads/headers/receiver_th.h"
#include "../threads/headers/display_th.h"
#include "../network/headers/transmitter.h"
#include "../library/headers/utilities.h"

GtkWidget* chat_window = NULL;
GtkWidget* send_button = NULL;
GtkWidget* send_entry  = NULL;
GtkWidget* msg_box = NULL;

GtkWidget* nip_entry = NULL;
GtkWidget* nport_entry = NULL;
GtkWidget* add_button = NULL;

pair* m_pair = NULL;

GtkWidget* create_label(char* msg,int size){
  char data[size + 1];
  memcpy(data,msg,size);
  data[size] = '\0';

  gchar* lab_msg = g_locale_to_utf8(data, -1, NULL, NULL, NULL);

  GtkWidget* label = gtk_label_new(lab_msg);
  g_free(lab_msg);

  gtk_label_set_xalign(GTK_LABEL(label),0.0);
  gtk_label_set_yalign(GTK_LABEL(label),0.0);

  gtk_widget_set_size_request (label,600, ((size / 140)+1) * 13 );
  gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);

  return label;
}

void add_msg_to_box (char* msg, int size) {
  if (msg_box == NULL)
    return;

  GtkWidget* label = create_label(msg, size);

  gtk_box_pack_start(GTK_BOX(msg_box),label, FALSE, FALSE, 2);
  gtk_widget_show_all(chat_window);
}

void on_send(GtkWidget *pButton, gpointer dat) {
    (void)pButton;
    (void)dat;

    const char* msg = gtk_entry_get_text(GTK_ENTRY(send_entry));

    int name_size = strlen(m_pair->pseudo);
    int data_size = strlen(msg);

    char data[name_size + data_size + 3];
    data[0] = '\0';
    strcat(data, m_pair->pseudo);
    strcat(data, ": ");
    strcat(data, msg);
    sendTLVData(m_pair, data);
    add_msg_to_box(data,strlen(data));
    display_msg_shell((unsigned char*)data, strlen(data));
    gtk_entry_set_text(GTK_ENTRY(send_entry), "");
}

void on_add(GtkWidget *pButton, gpointer dat) {
    (void)pButton;
    (void)dat;

    const char* ip_str_entry = gtk_entry_get_text(GTK_ENTRY(nip_entry));
    const char* port_str_entry = gtk_entry_get_text(GTK_ENTRY(nport_entry));

    char* ip = g_locale_from_utf8 (ip_str_entry,-1,NULL,NULL,NULL);
    char* port_str = g_locale_from_utf8 (port_str_entry,-1,NULL,NULL,NULL);

    short status = addPotNeighbString(m_pair,ip, port_str);
    if(status == false){
      perror("addPotneighbstring");
      return;
    }

    gtk_entry_set_text(GTK_ENTRY(nip_entry), "");
    gtk_entry_set_text(GTK_ENTRY(nport_entry), "");
}

/**
 * @brief Fonction permettant de "tuer" les threads et d'envoyer des GoAway prévénant
 *        notre départ à tous nos voisins lors de la fermeture de la fenêtre de chat.
 *
 * @return void
 */
void on_destroy_chat() {
  for(int i = 0; i < m_pair->neighbours-> size; i++){
    hashnode* ngb = m_pair->neighbours-> list[i];
    while(ngb != NULL){
      sendTLVGoAway(m_pair, 1, "BYE !", ngb->ip_port.ip, ngb->ip_port.port);
      ngb = ngb->next;
    }
  }

  pthread_kill(sender_t, SIGKILL);
  pthread_kill(reception_t, SIGKILL);
  pthread_kill(display_t, SIGKILL);
  gtk_main_quit();
}


void init_componant (GtkBuilder *builder){
  chat_window = GTK_WIDGET(gtk_builder_get_object(builder, "chat_window"));
  send_button = GTK_WIDGET(gtk_builder_get_object(builder, "send_button"));
  send_entry  = GTK_WIDGET(gtk_builder_get_object(builder, "send_entry"));
  msg_box = GTK_WIDGET(gtk_builder_get_object(builder, "box"));
  nip_entry = GTK_WIDGET(gtk_builder_get_object(builder, "ip_entry"));
  nport_entry = GTK_WIDGET(gtk_builder_get_object(builder, "port_entry"));
  add_button = GTK_WIDGET(gtk_builder_get_object(builder, "add_button"));
}

void init_signal() {
  g_signal_connect(G_OBJECT(chat_window), "destroy", G_CALLBACK(on_destroy_chat), NULL);
  g_signal_connect(G_OBJECT(send_button), "clicked", G_CALLBACK(on_send), NULL);
  g_signal_connect(G_OBJECT(add_button), "clicked", G_CALLBACK(on_add), NULL);
}

short display_chat_window(const char* pseudo, short port) {
  m_pair = create_pair(port,(char*)pseudo);
  if(m_pair == NULL) {
    return 0;
  }

  pthread_create(&reception_t, NULL, receiver_thread,m_pair);
  pthread_create(&sender_t, NULL, sender_thread, m_pair);
  pthread_create(&display_t, NULL,display_thread, m_pair);

  GtkBuilder *builder = gtk_builder_new();
  gtk_builder_add_from_file (builder, "interface/chat.glade", NULL);

  init_componant(builder);
  init_signal();

  gtk_builder_connect_signals(builder, NULL);
  g_object_unref(builder);
  GError **err = NULL;
  gtk_window_set_icon_from_file(GTK_WINDOW(chat_window), "interface/ICON_resized.png", err);
  gtk_widget_show(chat_window);
  return 1;
}
