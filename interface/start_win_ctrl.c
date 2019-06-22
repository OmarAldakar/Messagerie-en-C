#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "headers/chat_win_ctrl.h"
#include "headers/start_win_ctrl.h"

GtkWidget* start_window = NULL;
GtkWidget* pseudo_entry = NULL;
GtkWidget* port_entry = NULL;
GtkWidget* confirm_button = NULL;

short confirm_pressed = 0;


void on_destroy() {
  if (!confirm_pressed)
    gtk_main_quit();
}

void on_confirm(GtkWidget *pButton, gpointer data)
{
    (void)pButton;
    (void)data;
    confirm_pressed = 1;
    short port;
    const char* port_str = gtk_entry_get_text(GTK_ENTRY(port_entry));
    const char* pseudo = gtk_entry_get_text(GTK_ENTRY(pseudo_entry));

    char* copy_pseu = g_locale_from_utf8 (pseudo,-1,NULL,NULL,NULL);

    port = atoi(port_str); //IF FAIL NIMPORT QUELLE PORT SERA PRIS

    gtk_widget_destroy(start_window);
    display_chat_window(copy_pseu, port);
    g_free(copy_pseu);
}

void init_componant_st (GtkBuilder *builder){
  start_window = GTK_WIDGET(gtk_builder_get_object(builder, "start_window"));
  pseudo_entry = GTK_WIDGET(gtk_builder_get_object(builder, "pseudo"));
  port_entry = GTK_WIDGET(gtk_builder_get_object(builder, "port"));
  confirm_button = GTK_WIDGET(gtk_builder_get_object(builder, "confirmer"));
}

void init_signal_st() {
  g_signal_connect(G_OBJECT(start_window), "destroy", G_CALLBACK(on_destroy), NULL);
  g_signal_connect(confirm_button, "clicked", G_CALLBACK(on_confirm), NULL);
}

void display_start_window() {
  GtkBuilder* builder = gtk_builder_new();
  GtkCssProvider *cssProvider = gtk_css_provider_new ();

  gtk_css_provider_load_from_path(cssProvider,"interface/first_window.css",NULL);
  gtk_builder_add_from_file (builder, "interface/chat.glade", NULL);

  init_componant_st(builder);
  init_signal_st();

  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),GTK_STYLE_PROVIDER(cssProvider),GTK_STYLE_PROVIDER_PRIORITY_USER); //CSS
  gtk_builder_connect_signals(builder, NULL);
  g_object_unref(builder);
  GError **err = NULL;
  gtk_window_set_icon_from_file(GTK_WINDOW(start_window), "interface/ICON_resized.png", err);
  gtk_widget_show(start_window);
}
