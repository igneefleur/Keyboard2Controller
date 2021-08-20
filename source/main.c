#include "./graphics/window.c"

#include <stdio.h>

#define GET_VARIABLE_NAME(Variable) (#Variable)

void gtk_init(int * argc, char *** argv);


int main(int argc, char ** argv){
  //int keyboard = new_keyboard("/dev/input/by-path/platform-i8042-serio-0-event-kbd"); //  select keyboard
  /*
  int keyboard = new_keyboard("/dev/input/by-path/pci-0000:00:14.0-usb-0:1.4.4:1.0-event-kbd");
  int controller = new_controller();

  add_button(31, BTN_B, GET_VARIABLE_NAME(BTN_B));

  setup_buttons("./configurations/default");

  while (1) {
    struct key * _key = listen_keyboard(keyboard);
    if(_key != NULL)
      if(button_exist(_key->code)){
        struct button * _button = find_button(_key->code);
        printf("%s %s\n", _button->name, evval[_key->type]);
        //emit(controller, EV_KEY, _button->value, 1);
        emit(controller, EV_KEY, _button->value, _key->type);
      }
    remove_key(_key);
  }

  remove_button(find_button(31));

  stop_keyboard(keyboard);
  stop_controller(controller);

  */

  window(argc, argv);

  return 0;
}
