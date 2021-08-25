#include <stdio.h> // for malloc
#include <stdlib.h> // for free

#include "./../../librairies/uthash/src/uthash.h"
#include  <linux/uinput.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct key_to_button {
  int key; // key value

  int value; // like "BTN_B"
  int type; // "EV_KEY" or "EV_ABS"
  int force; // force to input

  UT_hash_handle hh; // makes this structure hashable
};

struct key_to_button * keys_to_buttons = NULL;

////////////////////////////////////////////////////////////////////////////////
//// METHODS

void add_key_to_button(int _key, int _value, int _type, int _force){
  struct key_to_button * _key_to_button = (struct key_to_button *)malloc(sizeof(struct key_to_button));

  _key_to_button->key = _key;

  _key_to_button->value = _value;
  _key_to_button->type = _type;
  _key_to_button->force = _force;

  HASH_ADD_INT(keys_to_buttons, key, _key_to_button);
}

int key_to_button_exist(int _key){
  struct key_to_button * _key_to_button;
  HASH_FIND_INT(keys_to_buttons, &_key, _key_to_button);
  if(_key_to_button) return 1;
  return 0;
}

struct key_to_button * find_key_to_button(int _key){
  struct key_to_button * _key_to_button;

  HASH_FIND_INT(keys_to_buttons, &_key, _key_to_button);
  return _key_to_button;
}

void remove_key_to_button(struct key_to_button * _key_to_button){
  HASH_DEL(keys_to_buttons, _key_to_button);
  free(_key_to_button);
}
