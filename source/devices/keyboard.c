// https://stackoverflow.com/questions/20943322/accessing-keys-from-linux-input-device

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <string.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static const char * const evval[3] = {
    "RELEASED",
    "PRESSED ",
    "REPEATED"
};

////////////////////////////////////////////////////////////////////////////////
//// KEYBOARD //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct key {
  int code;
  int type;
};

struct key * create_key(int _code, int _type){
  struct key * _key = (struct key *)malloc(sizeof(struct key));

  _key->code = _code;
  _key->type = _type;

  return _key;
}

void remove_key(struct key * k){
  free(k);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int new_keyboard(char * path){
  const char * dev = path;
  int fd;

  fd = open(dev, O_RDONLY);
  if (fd == -1){
    fprintf(stderr, "Cannot open %s: %s.\n", dev, strerror(errno));
    return EXIT_FAILURE;
  }

  return fd;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct key * listen_keyboard(int fd){
  struct input_event ev;
  ssize_t n;

  n = read(fd, &ev, sizeof ev);
  if (n == (ssize_t)-1){
    if (errno != EINTR)
      perror("don't work..");
  } else
  if (n != sizeof ev){
    errno = EIO;
    perror("don't work..");
  }

  if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2){
    //printf("%s ", evval[ev.value]);
    //printf("%s\n", (int)ev.code);
    return create_key((int)ev.code, ev.value);
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int stop_keyboard(int fd){
  close(fd);
}
