#include <stdio.h> // command malloc
#include <stdlib.h> // command free
#include <gtk/gtk.h> // gtkbutton

#include "./../../librairies/uthash/src/uthash.h"

struct window_button {
  char * name;

  GtkButton * gtk_button;

  GThread * main_thread;
  GThread * wait_thread;
  int thread_flag;

  int value;
  int type;
  int force;

  UT_hash_handle hh; // makes this structure hashable
};

struct window_button * window_buttons = NULL;

struct window_button * create_window_button(char * _name, GtkButton * _gtk_button, int _value, int _type, int _force){
  struct window_button * _window_button = (struct window_button *)malloc(sizeof(struct window_button));

  _window_button->name = _name;

  _window_button->gtk_button = _gtk_button;

  _window_button->thread_flag = 0;

  _window_button->value = _value;
  _window_button->type = _type;
  _window_button->force = _force;

  HASH_ADD_STR(window_buttons, name, _window_button);
  return _window_button;
}

void delete_window_button(struct window_button * _window_button){
  HASH_DEL(window_buttons, _window_button);
  free(_window_button);
}
