#include <dirent.h> // read directory
#include <stdio.h> // print
#include <time.h> // for calculate ti-/...

#include "./window_button.c"

#include "./../dictionaries/button_to_key.c"
#include "./../dictionaries/key_to_button.c"

#include "./../keyboard.c"
#include "./../controller.c"

#include <gtk/gtk.h>
#include <linux/uinput.h>
//#include <g.h>

#define STRING_SIZE 256
#define INT_SIZE ((CHAR_BIT * sizeof(int)) / 3 + 2)

////////////////////////////////////////////////////////////////////////////////
//// CONSTANTS

const char default_device[STRING_SIZE] = "No Device";

////////////////////////////////////////////////////////////////////////////////
//// VARIABLES

GMainContext * context;

GMutex mutex;

int controller;
GThread * controller_thread;
int controller_thread_flag = 0;

char actual_device[STRING_SIZE] = "No Device";

int keyboard;
char keyboard_path[STRING_SIZE];

////////////////////////////////////////////////////////////////////////////////
//// SIGNALS

void on_window_destroy(GtkWidget * _widget, gpointer user_data){
   printf("%s\n", "DESTROY PROGRAM");
  gtk_main_quit();
}

////////////////////////////////////////////////////////////////////////////////
//// DEVICE SELECTOR

struct device_selector {
  GtkBox * gtk_box;
  GtkAppChooserButton * gtk_app_chooser_button;
};

void setup_device_selector(GtkAppChooserButton * _device_selector){
  DIR * d;
  struct dirent * dir;
  d = opendir("/dev/input/by-path");

  dir = readdir(d); // skip "."
  dir = readdir(d); // skip ".."

  gtk_app_chooser_button_append_custom_item(_device_selector, default_device, default_device, NULL);
  gtk_app_chooser_button_set_active_custom_item(_device_selector, default_device);
  if(d){
    while ((dir = readdir(d)) != NULL) {
      gtk_app_chooser_button_append_custom_item(_device_selector, dir->d_name, dir->d_name, NULL);
    }
    closedir(d);
  }
}

void item_activated(GtkAppChooserButton * _device_selector, gchar* _device, gpointer _user_data){

  //stop_keyboard(keyboard);
  if(!strcmp(_device, default_device)){ // if default_device is selected
    memcpy(actual_device, default_device, STRING_SIZE);
    return;
  }

  char file[STRING_SIZE] = "/dev/input/by-path/";
  strcat(file, _device);

  keyboard = new_keyboard(file);
  memcpy(keyboard_path, file, STRING_SIZE); // "keyboard_path = file" C is shit
  memcpy(actual_device, _device, STRING_SIZE);
}

////////////////////////////////////////////////////////////////////////////////
//// CONTROLLER BUTTONS

struct anti_crash_button {
  GtkButton * gtk_button;
  char * code;
};

int anti_crash_button_wait_input(void * data){
  g_mutex_lock(&mutex);
  struct anti_crash_button * bwt = (struct anti_crash_button *) data; // sensitive (_window_button)

  //printf("%s\n", bwt->gtk_button);
  gtk_button_set_label(bwt->gtk_button, bwt->code); // ultra sensitive (_window_button)
  g_mutex_unlock(&mutex);
  return 0;
}


void * thread_button_wait_input(void * data){
  g_mutex_lock(&mutex);
  struct window_button * _window_button = (struct window_button *) data; // sensitive (_window_button)
  _window_button->thread_flag = 1; // sensitive (_window_button) // set flag to TRUE

  int temp_keyboard = new_keyboard(keyboard_path); // sensitive (keyboard_path)
  g_mutex_unlock(&mutex);

  while (1) {
    struct key * _key = listen_keyboard(temp_keyboard);

    g_mutex_lock(&mutex);
    if(!_window_button->thread_flag){ // sensitive (_window_button)
      g_mutex_unlock(&mutex);
      remove_key(_key); break;
    } else {
      g_mutex_unlock(&mutex);
      if(_key != NULL) if(_key->type == 1){
        //printf("%d %s\n", _key->code, evval[_key->type]); // sensitive ? (evval)
        char _code[INT_SIZE];
        sprintf(_code, "%d", _key->code);

        g_mutex_lock(&mutex);

        struct anti_crash_button _anti_crash_button = {.gtk_button = _window_button->gtk_button, .code = _code,};
        GSource * source = g_idle_source_new();
        g_source_set_callback(source, anti_crash_button_wait_input, &_anti_crash_button, NULL);
        g_source_attach(source, context);
        g_source_unref(source);

        if(button_to_key_exist(_window_button->name)) remove_button_to_key(find_button_to_key(_window_button->name)); // sensitive (_window_button)
        add_button_to_key(_window_button->name, _window_button->value, _window_button->type, _window_button->force, _key->code); // sensitive (_window_button)

        g_mutex_unlock(&mutex);

        remove_key(_key);
        break;
      }
    }
    remove_key(_key);
  }
  stop_keyboard(temp_keyboard);

  g_mutex_lock(&mutex);
  _window_button->thread_flag = 0; // sensitive (_window_button) // set flag to FALSE
  g_mutex_unlock(&mutex);

  return NULL;
}



int button_wait_time(void * data){
  g_mutex_lock(&mutex);
  struct window_button * _window_button = (struct window_button *) data; // sensitive (_window_button)

  gtk_button_set_label(_window_button->gtk_button, ""); // ultra sensitive (_window_button)
  g_mutex_unlock(&mutex);
  return 0;
}

void * thread_button_wait_time(void * data){
  g_mutex_lock(&mutex);
  struct window_button * _window_button = (struct window_button *) data; // sensitive (_window_button)
  g_mutex_unlock(&mutex);

  sleep(3);

  g_mutex_lock(&mutex);
  if (!_window_button->thread_flag){
    g_mutex_unlock(&mutex);
    return NULL;
  }

  if(button_to_key_exist(_window_button->name)) remove_button_to_key(find_button_to_key(_window_button->name)); // sensitive (_window_button)

  GSource * source = g_idle_source_new();
  g_source_set_callback(source, button_wait_time, _window_button, NULL);
  g_source_attach(source, context);
  g_source_unref(source);

  _window_button->thread_flag = 0; // sensitive (_window_button) // set flag to FALSE
  g_mutex_unlock(&mutex);

  printf("%s\n", "ABORD");
  return NULL;
}

void button_clicked(GtkButton * _button, gpointer data){
  if(!strcmp(actual_device, default_device)) return;

  g_mutex_lock(&mutex);
  struct window_button * _window_button = (struct window_button *) data; // sensitive (_window_button)

  if(_window_button->thread_flag){ // sensitive (_window_button)
    printf("%s\n", "SKIPPED");
    g_mutex_unlock(&mutex);
    return;
  }

  gtk_button_set_label(_button, "WAIT"); // sensitive (_button)
  g_mutex_unlock(&mutex);

  _window_button->main_thread = g_thread_new(NULL, thread_button_wait_input, _window_button);
  _window_button->wait_thread = g_thread_new(NULL, thread_button_wait_time, _window_button);

}

////////////////////////////////////////////////////////////////////////////////
//// CONTROLLER

void * start_controller(void * data){
  g_mutex_lock(&mutex);
  controller_thread_flag = 1;
  int _keyboard = keyboard; // sensitive (keyboard)
  g_mutex_unlock(&mutex);

  while (1) {
    struct key * _key = listen_keyboard(_keyboard);

    g_mutex_lock(&mutex);
    if(!controller_thread_flag){
      g_mutex_unlock(&mutex);
      return NULL;
    } else {
      g_mutex_unlock(&mutex);
      if(_key != NULL){
        g_mutex_lock(&mutex);
        if(key_to_button_exist(_key->code)){ // sensitive (key_to_button())
          struct key_to_button * _key_to_button = find_key_to_button(_key->code); // sensitive (_key_to_button, find_key_to_button())

          if(_key->type == 0) emit(controller, _key_to_button->type, _key_to_button->value, 0); // sensitive (controller, _key_to_button)
          else if(_key->type == 1) emit(controller, _key_to_button->type, _key_to_button->value, _key_to_button->force); // sensitive (controller, _key_to_button)

          //printf("KEY %s %s : SIMULATE %d\n", _key->code, evval[_key->type], _key_to_button->value); // sensitive (evval, _key_to_button)
          //printf("%s\n", "A BUTTON PRESSED");
        }
        g_mutex_unlock(&mutex);
      }
    }
    remove_key(_key);
  }
}

////////////////////////////////////////////////////////////////////////////////
//// APPLY BUTTON

void apply_button_clicked(GtkButton * _button, gpointer null){
  printf("%s\n", "APPLY START");
  struct button_to_key * _button_to_key;

  g_mutex_lock(&mutex);
  stop_controller(controller);

  for(_button_to_key = buttons_to_keys; _button_to_key != NULL; _button_to_key = _button_to_key->hh.next) {

    char _code[INT_SIZE]; sprintf(_code, "%d", _button_to_key->code); // transform int in string

    printf("  SET %s WITH KEY %s\n", _button_to_key->key, _code);
    if(key_to_button_exist(_button_to_key->code)) remove_key_to_button(find_key_to_button(_button_to_key->code));
    add_key_to_button(_button_to_key->code, _button_to_key->value, _button_to_key->type, _button_to_key->force);
  }

  controller = new_controller();
  controller_thread_flag = 0;
  g_mutex_unlock(&mutex);

  controller_thread = g_thread_new(NULL, start_controller, NULL);
  printf("%s\n", "APPLY DONE");
}

////////////////////////////////////////////////////////////////////////////////
//// CLEAR BUTTON

void clear_button_clicked(GtkButton * _button, gpointer null){
  printf("%s\n", "CLEAR START");
  struct button_to_key * _button_to_key;
  struct window_button * _window_button;

  g_mutex_lock(&mutex);
  for(_button_to_key = buttons_to_keys; _button_to_key != NULL; _button_to_key = _button_to_key->hh.next) {

    char _code[INT_SIZE]; sprintf(_code, "%d", _button_to_key->code); // transform int in string

    printf("  CLEAR %s\n", _button_to_key->key);
    remove_button_to_key(_button_to_key);
  }

  for (_window_button = window_buttons; _window_button != NULL; _window_button = _window_button->hh.next) {
    _window_button->thread_flag = 0;
    gtk_button_set_label(_window_button->gtk_button, "");
  }
  g_mutex_unlock(&mutex);

  printf("%s\n", "CLEAR DONE");
}

////////////////////////////////////////////////////////////////////////////////
//// REFRESH BUTTON

void refresh_button_clicked(GtkButton * _button, gpointer data){
  printf("%s\n", "REFRESH START");

  g_mutex_lock(&mutex);
  struct device_selector * _selector = (struct device_selector *) data;

  gtk_widget_destroy(GTK_WIDGET(_selector->gtk_app_chooser_button));
  _selector->gtk_app_chooser_button = GTK_APP_CHOOSER_BUTTON(gtk_app_chooser_button_new(""));
  gtk_container_add(GTK_CONTAINER(_selector->gtk_box), GTK_WIDGET(_selector->gtk_app_chooser_button));
  gtk_widget_show(GTK_WIDGET(_selector->gtk_app_chooser_button));
  setup_device_selector(_selector->gtk_app_chooser_button);
  g_signal_connect(G_OBJECT(_selector->gtk_app_chooser_button), "custom-item-activated", G_CALLBACK(item_activated), NULL);

  g_mutex_unlock(&mutex);

  clear_button_clicked(NULL, NULL);
  printf("%s\n", "REFRESH DONE");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void gtk_init(int * argc, char *** argv);

int window(int argc, char ** argv){

  GtkBuilder * builder;
  GtkWidget * window;

  struct device_selector * selector = (struct device_selector *)malloc(sizeof(struct device_selector));

  GtkButton * apply_button;
  GtkButton * clear_button;
  GtkButton * refresh_button;

  GtkButton * left_stick_up;
  GtkButton * left_stick_left;
  GtkButton * left_stick_right;
  GtkButton * left_stick_down;

  GtkButton * d_pad_up;
  GtkButton * d_pad_left;
  GtkButton * d_pad_right;
  GtkButton * d_pad_down;

  GtkButton * other_button_l;
  GtkButton * other_button_zl;
  GtkButton * other_button_minus;
  GtkButton * other_button_plus;
  GtkButton * other_button_r;
  GtkButton * other_button_zr;

  GtkButton * face_button_x;
  GtkButton * face_button_y;
  GtkButton * face_button_a;
  GtkButton * face_button_b;

  GtkButton * right_stick_up;
  GtkButton * right_stick_left;
  GtkButton * right_stick_right;
  GtkButton * right_stick_down;

////////////////////////////////////////////////////////////////////////////////
//// SETUP GTK + GLADE

  gtk_init(&argc, &argv); // initialize GTK
  g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", TRUE, NULL); // because png controller istk_widget_destroy(GTK white (and i'm lazy to change according to the theme)

  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "./glade/config.glade", NULL);
  gtk_builder_connect_signals(builder, NULL);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

  selector->gtk_box = GTK_BOX(gtk_builder_get_object(builder, "selector_container"));
  selector->gtk_app_chooser_button = GTK_APP_CHOOSER_BUTTON(gtk_builder_get_object(builder, "selector_control"));
  setup_device_selector(selector->gtk_app_chooser_button);
  g_signal_connect(G_OBJECT(selector->gtk_app_chooser_button), "custom-item-activated", G_CALLBACK(item_activated), NULL);

  apply_button = GTK_BUTTON(gtk_builder_get_object(builder, "apply"));
  g_signal_connect(G_OBJECT(apply_button), "clicked", G_CALLBACK(apply_button_clicked), NULL);

  clear_button = GTK_BUTTON(gtk_builder_get_object(builder, "clear"));
  g_signal_connect(G_OBJECT(clear_button), "clicked", G_CALLBACK(clear_button_clicked), NULL);

  refresh_button = GTK_BUTTON(gtk_builder_get_object(builder, "refresh"));
  g_signal_connect(G_OBJECT(refresh_button), "clicked", G_CALLBACK(refresh_button_clicked), selector);

  context = g_main_context_default();

////////////////////////////////////////////////////////////////////////////////
//// SETUP CONTROLLER

  controller = new_controller();
  controller_thread = g_thread_new(NULL, start_controller, NULL);

////////////////////////////////////////////////////////////////////////////////
//// SETUP ALL BUTTONS

//// LEFT STICK

  left_stick_up = GTK_BUTTON(gtk_builder_get_object(builder, "left_stick_up"));
  struct window_button * abs_y_positive = create_window_button("ABS_Y_POSITIVE", left_stick_up, ABS_Y, EV_ABS, 512);
  g_signal_connect(G_OBJECT(left_stick_up), "pressed", G_CALLBACK(button_clicked), abs_y_positive);

  left_stick_left = GTK_BUTTON(gtk_builder_get_object(builder, "left_stick_left"));
  struct window_button * abs_x_negative = create_window_button("ABS_X_NEGATIVE", left_stick_left, ABS_X, EV_ABS, -512);
  g_signal_connect(G_OBJECT(left_stick_left), "pressed", G_CALLBACK(button_clicked), abs_x_negative);

  left_stick_right = GTK_BUTTON(gtk_builder_get_object(builder, "left_stick_right"));
  struct window_button * abs_x_positive = create_window_button("ABS_X_POSITIVE", left_stick_right, ABS_X, EV_ABS, 512);
  g_signal_connect(G_OBJECT(left_stick_right), "pressed", G_CALLBACK(button_clicked), abs_x_positive);

  left_stick_down = GTK_BUTTON(gtk_builder_get_object(builder, "left_stick_down"));
  struct window_button * abs_y_negative = create_window_button("ABS_Y_NEGATIVE", left_stick_down, ABS_Y, EV_ABS, -512);
  g_signal_connect(G_OBJECT(left_stick_down), "pressed", G_CALLBACK(button_clicked), abs_y_negative);

//// D-PAD

  d_pad_up = GTK_BUTTON(gtk_builder_get_object(builder, "d_pad_up"));
  struct window_button * btn_dpad_up = create_window_button("BTN_DPAD_UP", d_pad_up, BTN_DPAD_UP, EV_KEY, 1);
  g_signal_connect(G_OBJECT(d_pad_up), "pressed", G_CALLBACK(button_clicked), btn_dpad_up);

  d_pad_left = GTK_BUTTON(gtk_builder_get_object(builder, "d_pad_left"));
  struct window_button * btn_dpad_left = create_window_button("BTN_DPAD_LEFT", d_pad_left, BTN_DPAD_LEFT, EV_KEY, 1);
  g_signal_connect(G_OBJECT(d_pad_left), "pressed", G_CALLBACK(button_clicked), btn_dpad_left);

  d_pad_right = GTK_BUTTON(gtk_builder_get_object(builder, "d_pad_right"));
  struct window_button * btn_dpad_right = create_window_button("BTN_DPAD_RIGHT", d_pad_right, BTN_DPAD_RIGHT, EV_KEY, 1);
  g_signal_connect(G_OBJECT(d_pad_right), "pressed", G_CALLBACK(button_clicked), btn_dpad_right);

  d_pad_down = GTK_BUTTON(gtk_builder_get_object(builder, "d_pad_down"));
  struct window_button * btn_dpad_down = create_window_button("BTN_DPAD_DOWN", d_pad_down, BTN_DPAD_DOWN, EV_KEY, 1);
  g_signal_connect(G_OBJECT(d_pad_down), "pressed", G_CALLBACK(button_clicked), btn_dpad_down);

//// OTHER BUTTONS

   other_button_l = GTK_BUTTON(gtk_builder_get_object(builder, "other_button_l"));
  struct window_button * btn_tl = create_window_button("BTN_TL", other_button_l, BTN_TL, EV_KEY, 1);
  g_signal_connect(G_OBJECT(other_button_l), "pressed", G_CALLBACK(button_clicked), btn_tl);

  other_button_zl = GTK_BUTTON(gtk_builder_get_object(builder, "other_button_zl"));
  struct window_button * btn_tl2 = create_window_button("BTN_TL2", other_button_zl, BTN_TL2, EV_KEY, 1);
  g_signal_connect(G_OBJECT(other_button_zl), "pressed", G_CALLBACK(button_clicked), btn_tl2);


  other_button_minus = GTK_BUTTON(gtk_builder_get_object(builder, "other_button_minus"));
  struct window_button * btn_start = create_window_button("BTN_START", other_button_minus, BTN_START, EV_KEY, 1);
  g_signal_connect(G_OBJECT(other_button_minus), "pressed", G_CALLBACK(button_clicked), btn_start);

  other_button_plus = GTK_BUTTON(gtk_builder_get_object(builder, "other_button_plus"));
  struct window_button * btn_select = create_window_button("BTN_SELECT", other_button_plus, BTN_SELECT, EV_KEY, 1);
  g_signal_connect(G_OBJECT(other_button_plus), "pressed", G_CALLBACK(button_clicked), btn_select);


  other_button_r = GTK_BUTTON(gtk_builder_get_object(builder, "other_button_r"));
  struct window_button * btn_tr = create_window_button("BTN_TR", other_button_r, BTN_TR, EV_KEY, 1);
  g_signal_connect(G_OBJECT(other_button_r), "pressed", G_CALLBACK(button_clicked), btn_tr);

  other_button_zr = GTK_BUTTON(gtk_builder_get_object(builder, "other_button_zr"));
  struct window_button * btn_tr2 = create_window_button("BTN_TR2", other_button_zr, BTN_TR2, EV_KEY, 1);
  g_signal_connect(G_OBJECT(other_button_zr), "pressed", G_CALLBACK(button_clicked), btn_tr2);

//// FACES BUTTONS

  face_button_x = GTK_BUTTON(gtk_builder_get_object(builder, "face_button_x"));
  struct window_button * btn_x = create_window_button("BTN_X",face_button_x,  BTN_X, EV_KEY, 1);
  g_signal_connect(G_OBJECT(face_button_x), "pressed", G_CALLBACK(button_clicked), btn_x);

  face_button_y = GTK_BUTTON(gtk_builder_get_object(builder, "face_button_y"));
  struct window_button * btn_y = create_window_button("BTN_Y", face_button_y, BTN_Y, EV_KEY, 1);
  g_signal_connect(G_OBJECT(face_button_y), "pressed", G_CALLBACK(button_clicked), btn_y);

  face_button_a = GTK_BUTTON(gtk_builder_get_object(builder, "face_button_a"));
  struct window_button * btn_a = create_window_button("BTN_A", face_button_a, BTN_A, EV_KEY, 1);
  g_signal_connect(G_OBJECT(face_button_a), "pressed", G_CALLBACK(button_clicked), btn_a);

  face_button_b = GTK_BUTTON(gtk_builder_get_object(builder, "face_button_b"));
  struct window_button * btn_b = create_window_button("BTN_B", face_button_b, BTN_B, EV_KEY, 1);
  g_signal_connect(G_OBJECT(face_button_b), "pressed", G_CALLBACK(button_clicked), btn_b);

//// RIGHT STICK

  right_stick_up = GTK_BUTTON(gtk_builder_get_object(builder, "right_stick_up"));
  right_stick_left = GTK_BUTTON(gtk_builder_get_object(builder, "right_stick_left"));
  right_stick_right = GTK_BUTTON(gtk_builder_get_object(builder, "right_stick_right"));
  right_stick_down = GTK_BUTTON(gtk_builder_get_object(builder, "right_stick_down"));


////////////////////////////////////////////////////////////////////////////////
//// START WINDOW

  g_object_unref(builder);
  gtk_widget_show(window);
  gtk_main();

////////////////////////////////////////////////////////////////////////////////
//// FREE CONTROLLER

  stop_controller(controller);

////////////////////////////////////////////////////////////////////////////////
//// FREE BUTTONS

// too laazy


  return EXIT_SUCCESS;
}
