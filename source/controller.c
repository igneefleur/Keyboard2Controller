#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include  <linux/uinput.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void emit(int fd, int type, int code, int val){
  struct input_event ie[2];
  memset(&ie, 0, sizeof ie);

  ie[0].type = type;
  ie[0].code = code;
  ie[0].value = val;
  // timestamp values below are ignored
  ie[0].time.tv_sec = 0;
  ie[0].time.tv_usec = 0;

  // maybe useless
  ie[1].type = EV_SYN;
  ie[1].code = SYN_REPORT;
  ie[1].value = 0;

  write(fd, &ie, sizeof(ie));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// enable and configure an absolute analog channel
static void setup_abs(int fd, unsigned chan, int min, int max){
  if (ioctl(fd, UI_SET_ABSBIT, chan))
    perror("UI_SET_ABSBIT");

  struct uinput_abs_setup s = {
     .code = chan,
     .absinfo = { .minimum = min,  .maximum = max },
    };

  if (ioctl(fd, UI_ABS_SETUP, &s))
    perror("UI_ABS_SETUP");
}

////////////////////////////////////////////////////////////////////////////////
//// CONTROLLER ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int new_controller(void){
  struct uinput_setup setup = {
    .name = "JoyTest",
    .id = {
      .bustype = BUS_USB,
      .vendor  = 0x1234,
      .product = 0x5678,
      .version = 2,
    }
  };

  int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (fd < 0){
    perror("open /dev/uinput");
    return 1;
  }

// BUTTONS
  ioctl(fd, UI_SET_EVBIT, EV_KEY);

  ioctl(fd, UI_SET_KEYBIT, BTN_A);
  ioctl(fd, UI_SET_KEYBIT, BTN_B);
  ioctl(fd, UI_SET_KEYBIT, BTN_X);
  ioctl(fd, UI_SET_KEYBIT, BTN_Y);

  ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_UP);
  ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_DOWN);
  ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_LEFT);
  ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_RIGHT);

  ioctl(fd, UI_SET_KEYBIT, BTN_TL);
  ioctl(fd, UI_SET_KEYBIT, BTN_TR);
  ioctl(fd, UI_SET_KEYBIT, BTN_TL2);
  ioctl(fd, UI_SET_KEYBIT, BTN_TR2);

  ioctl(fd, UI_SET_KEYBIT, BTN_START);
  ioctl(fd, UI_SET_KEYBIT, BTN_SELECT);

  //ioctl(fd, UI_SET_KEYBIT, BTN_THUMBL);
  //ioctl(fd, UI_SET_KEYBIT, BTN_THUMBR);

// JOYSTICKS
  ioctl(fd, UI_SET_EVBIT, EV_ABS);

  setup_abs(fd, ABS_X,  -512, 512);
  setup_abs(fd, ABS_Y,  -512, 512);
  //setup_abs(fd, ABS_Z,  -32767, 32767);

  //setup_abs(fd, ABS_RX, 0, 100);
  //setup_abs(fd, ABS_RY, 0, 100);
  //setup_abs(fd, ABS_RZ, 0, 100);

  if (ioctl(fd, UI_DEV_SETUP, &setup)){
    perror("UI_DEV_SETUP");
    return 1;
  }

  if (ioctl(fd, UI_DEV_CREATE)){ // generate controller
    perror("UI_DEV_CREATE");
    return 1;
  }

  return fd;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int stop_controller(int fd){
  if(ioctl(fd, UI_DEV_DESTROY)){
    printf("UI_DEV_DESTROY");
    return 1;
  }

  close(fd);
  return 0;
}
