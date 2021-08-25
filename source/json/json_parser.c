#include "./../../librairies/jsmn/jsmn.h"

#include <stdlib.h>

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

void json_parse(char * string_json){ // needs a fucking optimization
  char string_array[JSON_SIZE];
  char string_button_to_key[JSON_SIZE];

  jsmn_parser main_parser;
  jsmntok_t main_tokens[128];
  int jsmn_main_size;

  jsmn_parser array_parser;
  jsmntok_t array_tokens[128];
  int jsmn_array_size;

  jsmn_parser button_to_key_parser;
  jsmntok_t button_to_key_tokens[128];
  int jsmn_button_to_key_size;

  char temp_string[STRING_SIZE];
  char * key;
  int value;
  int type;
  int force;
  int code;

  jsmn_init(&main_parser);
  jsmn_main_size = jsmn_parse(&main_parser, string_json, strlen(string_json), main_tokens, sizeof(main_tokens) / sizeof(main_tokens[0]));

  for(int i = 0; i < jsmn_main_size; i++){
    if(jsoneq(string_json, &main_tokens[i], "buttons_to_keys") == 0){
      sprintf(string_array, "%.*s", main_tokens[i + 1].end - main_tokens[i + 1].start, string_json + main_tokens[i + 1].start);

      jsmn_init(&array_parser);
      jsmn_array_size = jsmn_parse(&array_parser, string_array, strlen(string_array), array_tokens, sizeof(array_tokens) / sizeof(array_tokens[0]));

      for(int j = 0; j < jsmn_array_size; j++){
        if(array_tokens[j].type == 1){ // test if tokens[j] has type OBJECT
          sprintf(string_button_to_key, "%.*s", array_tokens[j].end - array_tokens[j].start, string_array + array_tokens[j].start);

          jsmn_init(&button_to_key_parser);
          jsmn_button_to_key_size = jsmn_parse(&button_to_key_parser, string_button_to_key, strlen(string_button_to_key), button_to_key_tokens, sizeof(button_to_key_tokens) / sizeof(button_to_key_tokens[0]));

          for(int k = 1; k < jsmn_button_to_key_size; k++){
            if(jsoneq(string_button_to_key, &button_to_key_tokens[k], "key") == 0){
              sprintf(temp_string, "%.*s", button_to_key_tokens[k + 1].end - button_to_key_tokens[k + 1].start, string_button_to_key + button_to_key_tokens[k + 1].start);
              key = (char *) malloc((STRING_SIZE + 1) * sizeof(char));
              key = strcpy(key, temp_string);
            } else if(jsoneq(string_button_to_key, &button_to_key_tokens[k], "value") == 0){
              sprintf(temp_string, "%.*s", button_to_key_tokens[k + 1].end - button_to_key_tokens[k + 1].start, string_button_to_key + button_to_key_tokens[k + 1].start);
              value = atoi(temp_string);
            } else if(jsoneq(string_button_to_key, &button_to_key_tokens[k], "type") == 0){
              sprintf(temp_string, "%.*s", button_to_key_tokens[k + 1].end - button_to_key_tokens[k + 1].start, string_button_to_key + button_to_key_tokens[k + 1].start);
              type = atoi(temp_string);
            } else if(jsoneq(string_button_to_key, &button_to_key_tokens[k], "force") == 0){
              sprintf(temp_string, "%.*s", button_to_key_tokens[k + 1].end - button_to_key_tokens[k + 1].start, string_button_to_key + button_to_key_tokens[k + 1].start);
              force = atoi(temp_string);
            } else if(jsoneq(string_button_to_key, &button_to_key_tokens[k], "code") == 0){
              sprintf(temp_string, "%.*s", button_to_key_tokens[k + 1].end - button_to_key_tokens[k + 1].start, string_button_to_key + button_to_key_tokens[k + 1].start);
              code = atoi(temp_string);
            }
          }
          add_button_to_key(key, value, type, force, code);
        }
      }
    }
  }
}
