#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

#include <arpa/inet.h>
#include <err.h>
#include <fcntl.h>
#include <pthread.h>
#include <sysexits.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include "crc8.h"
#include "protocol.h"

static int fd;
static int has_odometer;
static uint16_t motor0_odometer, motor1_odometer;
static int32_t motor0_distance, motor1_distance;
static uint32_t messages_received;

static uint32_t motor0_requested_speed, motor1_requested_speed;
static int reverse;

static pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool display_dirty = true;
static pthread_cond_t display_dirty_cond = PTHREAD_COND_INITIALIZER;

struct var {
  uint16_t value;
  struct timeval last_updated;
};

static const struct {
  uint8_t id;
  const char *name;
} kVarNames[] = {
  { VAR_MOTOR0_INVALID_TRANSITIONS, "motor #0 invalid transitions" },
  { VAR_MOTOR0_INVALID_STATES, "motor #0 invalid states" },
  { VAR_MOTOR1_INVALID_TRANSITIONS, "motor #1 invalid transitions" },
  { VAR_MOTOR1_INVALID_STATES, "motor #1 invalid states" },
};

static std::map<uint8_t, var> vars;

static void write_all(int fd, const void *buffer, size_t size) {
  size_t offset = 0;

  while (offset < size) {
    ssize_t ret;

    ret = write(fd, (const char *)buffer + offset, size - offset);

    if (ret < 0)
      err(EXIT_FAILURE, "Failed to write %zu bytes", size - offset);
    else if (!ret)
      errx(EXIT_FAILURE, "EOF while attempting to write %zu bytes",
           size - offset);

    offset += ret;
  }
}

static void *reader_thread(void *arg) {
  for (;;) {
    struct motor_message message;
    size_t fill = 0;

    while (fill < sizeof(message)) {
      if (1 != read(fd, (char *)&message + fill, 1))
        err(EXIT_FAILURE, "Read error");

      ++fill;

      if (fill == 1 && message.sync[0] != MOTOR_SYNC_BYTE0)
        fill = 0;
      else if (fill == 2 && message.sync[1] != MOTOR_SYNC_BYTE1)
        fill = 0;
    }

    if (message.crc8 !=
        crc8(&message.type,
             sizeof(message) - offsetof(struct motor_message, type))) {
      fill = 0;

      continue;
    }

    pthread_mutex_lock(&display_mutex);

    switch (message.type) {
      case MOTOR_MSG_ODOMETER: {

        if (!has_odometer &&
            message.u.odometer.motor0_odometer == motor0_odometer &&
            message.u.odometer.motor0_odometer == motor1_odometer)
          has_odometer = 1;

        int32_t left_delta =
          (int32_t) message.u.odometer.motor0_odometer - motor0_odometer;
        int32_t right_delta =
          (int32_t) message.u.odometer.motor1_odometer - motor1_odometer;
        motor0_odometer = message.u.odometer.motor0_odometer;
        motor1_odometer = message.u.odometer.motor1_odometer;

        /* Handle 16 bit arithmetic overflow.  */
        if (left_delta <= -0x8000)
          left_delta += 0x10000;
        else if (left_delta >= 0x8000)
          left_delta -= 0x10000;
        if (right_delta <= -0x8000)
          right_delta += 0x10000;
        else if (right_delta >= 0x8000)
          right_delta -= 0x10000;

        motor0_distance += left_delta;
        motor1_distance += right_delta;

      } break;

      case MOTOR_MSG_VAR:

        vars[message.u.var.id].value = message.u.var.value;
        gettimeofday(&vars[message.u.var.id].last_updated, nullptr);

        break;
    }

    ++messages_received;
    display_dirty = true;

    pthread_cond_broadcast(&display_dirty_cond);

    pthread_mutex_unlock(&display_mutex);

    fill = 0;
  }
}

static void *user_input_thread(void *arg) {
  int ch;

  while (EOF != (ch = getchar())) {
    if (ch == 'r') {
      motor0_distance = 0;
      motor1_distance = 0;
      continue;
    }

    struct motor_message msg;

    msg.sync[0] = MOTOR_SYNC_BYTE0;
    msg.sync[1] = MOTOR_SYNC_BYTE1;

    switch (ch) {
      case '-':

        msg.type = MOTOR_MSG_REQUEST_SPEED;
        reverse = !reverse;

        break;

      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':

        msg.type = MOTOR_MSG_REQUEST_SPEED;
        motor0_requested_speed = (ch - '0') * 10000 / 10;
        motor1_requested_speed = (ch - '0') * 10000 / 10;

        break;

      case '0':
      case ' ':

        msg.type = MOTOR_MSG_REQUEST_SPEED;
        motor0_requested_speed = 0;
        motor1_requested_speed = 0;

        break;

      case 'v':
      case 'V':

        msg.type = MOTOR_MSG_REQUEST_VARS;

        break;
    }

    if (msg.type == MOTOR_MSG_REQUEST_SPEED) {
      if (reverse) {
        msg.u.speed.motor0_speed = -motor0_requested_speed;
        msg.u.speed.motor1_speed = -motor1_requested_speed;
      } else {
        msg.u.speed.motor0_speed = motor0_requested_speed;
        msg.u.speed.motor1_speed = motor1_requested_speed;
      }
    }

    msg.crc8 =
        crc8(&msg.type, sizeof(msg) - offsetof(struct motor_message, type));

    write_all(fd, &msg, sizeof(msg));

    motor0_requested_speed = msg.u.speed.motor0_speed;
    motor1_requested_speed = msg.u.speed.motor1_speed;
  }

  return NULL;
}

int main(int argc, char **argv) {
  struct termios tty;

  if (argc != 2) errx(EX_USAGE, "Usage: %s TTY", argv[0]);

  if (-1 == (fd = open(argv[1], O_RDWR | O_NOCTTY)))
    err(EXIT_FAILURE, "Failed to open '%s' in read/write mode", argv[1]);

  if (-1 == tcflush(fd, TCIOFLUSH)) err(EXIT_FAILURE, "tcflush failed");

  memset(&tty, 0, sizeof tty);

  cfsetospeed(&tty, B115200);
  cfsetispeed(&tty, B115200);
  cfmakeraw(&tty);

  if (-1 == tcsetattr(fd, TCSANOW, &tty))
    err(EXIT_FAILURE, "tcsetattr failed on '%s'", argv[1]);

  /* Disable input buffering on standard input.  */
  if (-1 == tcgetattr(STDIN_FILENO, &tty))
    err(EXIT_FAILURE, "tcgetattr failed on stdin");
  tty.c_lflag &= ~(ICANON | ECHO);
  if (-1 == tcsetattr(STDIN_FILENO, TCSANOW, &tty))
    err(EXIT_FAILURE, "tcsetattr failed on stdin");

  pthread_t reader_thread_handle;
  pthread_create(&reader_thread_handle, NULL, reader_thread, NULL);
  pthread_detach(reader_thread_handle);

  pthread_t user_input_thread_handle;
  pthread_create(&user_input_thread_handle, NULL, user_input_thread, NULL);
  pthread_detach(user_input_thread_handle);

  for (;;) {
    pthread_mutex_lock(&display_mutex);
    while (!display_dirty)
      pthread_cond_wait(&display_dirty_cond, &display_mutex);
    display_dirty = false;
    pthread_mutex_unlock(&display_mutex);

    printf("\033[H\033[2J");

    if (has_odometer) {
      printf("Odometer A: %d (%04x)\n", motor0_distance, motor0_odometer);
      printf("Odometer B: %d (%04x)\n", motor1_distance, motor1_odometer);
    } else
      printf("Missing odometry\n\n");

    printf("Messages received: %u\n", messages_received);

    printf("Motor A requested speed: %u\n", motor0_requested_speed);
    printf("Motor B requested speed: %u\n", motor1_requested_speed);

    struct timeval now;
    gettimeofday(&now, nullptr);

    for (const auto &v : vars) {
      const char *name = "unknown";

      for (const auto &n : kVarNames) {
        if (n.id == v.first) {
          name = n.name;
          break;
        }
      }

      printf("Var %-28s: %u (%.3f seconds ago)\n", name, v.second.value,
             (now.tv_sec - v.second.last_updated.tv_sec) +
                 1.0e-7 * (now.tv_usec - v.second.last_updated.tv_usec));
    }

    usleep(20000);
  }

  return EXIT_SUCCESS;
}
