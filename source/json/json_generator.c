#include "./../../librairies/json-maker/json-maker.c"

char * json_button_to_key(char * _dest, char const * _name, struct button_to_key const * _button_to_key){
  _dest = json_objOpen(_dest, _name);

  _dest = json_str(_dest, "key", _button_to_key->key);
  _dest = json_int(_dest, "value", _button_to_key->value);
  _dest = json_int(_dest, "type", _button_to_key->type);
  _dest = json_int(_dest, "force", _button_to_key->force);
  _dest = json_int(_dest, "code", _button_to_key->code);

  _dest = json_objClose(_dest);
  return _dest;
}

char * json_data(char * _dest, char const * _name){
  _dest = json_objOpen(_dest, _name);
  _dest = json_arrOpen(_dest, "buttons_to_keys");

  struct button_to_key * _button_to_key;
  for(_button_to_key = buttons_to_keys; _button_to_key != NULL; _button_to_key = _button_to_key->hh.next){
    _dest = json_button_to_key(_dest, NULL, _button_to_key);
  }

  _dest = json_arrClose(_dest);
  _dest = json_objClose(_dest);
  return _dest;
}

int data_to_json(char * _dest){
    char * p = json_data(_dest, NULL);
    p = json_end(p);
    return p - _dest;
}

char * json_generate(){
  char buff[JSON_SIZE];
  int len = data_to_json(buff);
  if(len >= sizeof buff) {
      fprintf(stderr, "%s%d%s%d\n", "Error. Len: ", len, " Max: ", (int)sizeof buff - 1);
      return "";
  }

  char * string = buff;
  return string;
}
