#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <err.h>
#include <fcntl.h>
#include <pthread.h>
#include <sysexits.h>
#include <termios.h>
#include <unistd.h>

#include "crc8.h"
#include "protocol.h"

static int fd;
static int has_odometer;
static uint32_t motor0_odometer, motor1_odometer;
static uint32_t messages_received;

static uint32_t motor0_requested_speed, motor1_requested_speed;

static pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;
static int display_dirty = 1;
static pthread_cond_t display_dirty_cond = PTHREAD_COND_INITIALIZER;

static void
write_all (int fd, const void *buffer, size_t size)
{
  size_t offset = 0;

  while (offset < size)
    {
      ssize_t ret;

      ret = write (fd, (const char *) buffer + offset, size - offset);

      if (ret < 0)
        err(EXIT_FAILURE, "Failed to write %zu bytes", size - offset);
      else if (!ret)
        errx(EXIT_FAILURE, "EOF while attempting to write %zu bytes", size - offset);

      offset += ret;
    }
}

static void *
reader_thread (void *arg)
{
  for (;;)
    {
      struct motor_message message;
      size_t fill = 0;

      while (fill < sizeof (message))
        {
          if (1 != read(fd, (char *) &message + fill, 1))
            err(EXIT_FAILURE, "Read error");

          ++fill;

          if (fill == 1 && message.sync[0] != MOTOR_SYNC_BYTE0)
            fill = 0;
          else if (fill == 2 && message.sync[1] != MOTOR_SYNC_BYTE1)
            fill = 0;
        }

      pthread_mutex_lock (&display_mutex);

      switch (message.type)
        {
        case MOTOR_MSG_ODOMETER:

          motor0_odometer = message.u.odometer.motor0_odometer;
          motor1_odometer = message.u.odometer.motor1_odometer;
          has_odometer = 1;

          break;
        }

      ++messages_received;
      display_dirty = 1;

      pthread_cond_broadcast (&display_dirty_cond);

      pthread_mutex_unlock (&display_mutex);


      fill = 0;
    }
}

static void *
user_input_thread (void *arg)
{
  int ch;

  while (EOF != (ch = getchar ()))
    {
      struct motor_message msg;

      msg.sync[0] = MOTOR_SYNC_BYTE0;
      msg.sync[1] = MOTOR_SYNC_BYTE1;
      msg.type = MOTOR_MSG_REQUEST_SPEED;

      switch (ch)
        {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':

          msg.u.speed.motor0_speed = (ch - '0') * 127 / 9;
          msg.u.speed.motor1_speed = (ch - '0') * 127 / 9;

          break;

        default:
        case ' ':

          msg.u.speed.motor0_speed = 0;
          msg.u.speed.motor1_speed = 0;

          break;
        }

      msg.crc8 = crc8(&msg.type, sizeof(msg) - offsetof(struct motor_message, type));

      write_all (fd, &msg, sizeof(msg));

      motor0_requested_speed = msg.u.speed.motor0_speed;
      motor1_requested_speed = msg.u.speed.motor1_speed;
    }

  return NULL;
}

int
main (int argc, char **argv)
{
  struct termios tty;

  if (argc != 2)
    errx(EX_USAGE, "Usage: %s TTY", argv[0]);

  if (-1 == (fd = open(argv[1], O_RDWR | O_NOCTTY)))
    err(EXIT_FAILURE, "Failed to open '%s' in read/write mode", argv[1]);

  if (-1 == tcflush(fd, TCIOFLUSH))
    err(EXIT_FAILURE, "tcflush failed");

  memset (&tty, 0, sizeof tty);

  if (-1 == tcgetattr (fd, &tty))
    err(EXIT_FAILURE, "tcgetattr failed");

  cfsetospeed (&tty, 115200);
  cfsetispeed (&tty, 115200);
  cfmakeraw (&tty);
  tty.c_cflag |= CLOCAL | CREAD | CS8;

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
    err(EXIT_FAILURE, "tcsetattr failed");

  /* Disable input buffering on standard input.  */
  tcgetattr(STDIN_FILENO, &tty);
  tty.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW,&tty);

  pthread_t reader_thread_handle;
  pthread_create (&reader_thread_handle, NULL, reader_thread, NULL);
  pthread_detach (reader_thread_handle);

  pthread_t user_input_thread_handle;
  pthread_create (&user_input_thread_handle, NULL, user_input_thread, NULL);
  pthread_detach (user_input_thread_handle);

  for (;;)
    {
      pthread_mutex_lock (&display_mutex);
      while (!display_dirty)
        pthread_cond_wait (&display_dirty_cond, &display_mutex);
      display_dirty = 0;
      pthread_mutex_unlock (&display_mutex);

      printf ("\033[H\033[2J");

      if (has_odometer)
        {
          printf ("Odomoter A: %04x\n", motor0_odometer);
          printf ("Odometer B: %04x\n", motor1_odometer);
        }
      else
        printf ("Missing odometry\n\n");

      printf ("Messages received: %u\n", messages_received);

      printf ("Motor A requested speed: %u\n", motor0_requested_speed);
      printf ("Motor B requested speed: %u\n", motor1_requested_speed);

      usleep (20000);
    }

  return EXIT_SUCCESS;
}
