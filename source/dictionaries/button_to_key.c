#include <stdio.h> // for malloc
#include <stdlib.h> // for free

#include <pthread.h>
#include "./../../librairies/uthash/src/uthash.h"

struct button_to_key {
  char * key; // button name

  int value; // like "BTN_B"
  int type; // "EV_KEY" or "EV_ABS"
  int force; // force to input

  int code; // keyboard key

  UT_hash_handle hh; // make this structure hashable
};

////////////////////////////////////////////////////////////////////////////////
//// METHODS

struct button_to_key * buttons_to_keys = NULL;

void add_button_to_key(char * _key, int _value, int _type, int _force, int _code){
  struct button_to_key * _button_to_key = (struct button_to_key *)malloc(sizeof(struct button_to_key));

  _button_to_key->key = _key;

  _button_to_key->value = _value;
  _button_to_key->type = _type;
  _button_to_key->force = _force;

  _button_to_key->code = _code;

  HASH_ADD_STR(buttons_to_keys, key, _button_to_key);
}

int button_to_key_exist(char * _key){
  struct button_to_key * _button_to_key;
  HASH_FIND_STR(buttons_to_keys, _key, _button_to_key);
  if(_button_to_key) return 1;
  return 0;
}

struct button_to_key * find_button_to_key(char * _key){
  struct button_to_key * _button_to_key;
  HASH_FIND_STR(buttons_to_keys, _key, _button_to_key);
  return _button_to_key;
}

void remove_button_to_key(struct button_to_key * _button_to_key){
  HASH_DEL(buttons_to_keys, _button_to_key);
  free(_button_to_key);
}
