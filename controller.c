#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <err.h>
#include <fcntl.h>
#include <sysexits.h>
#include <termios.h>
#include <unistd.h>

#include "protocol.h"

#define TTY "/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_AE00DE7L-if00-port0"

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

static void
process_response (struct motor_response *resp)
{
  switch (resp->type)
    {
    case MOTOR_RESP_HELLO:

      fprintf (stderr, "Hello\n");

      break;

    case MOTOR_RESP_ERROR:

      fprintf (stderr, "Error: %d\n", resp->u.error.reserved0);

      break;

    case MOTOR_RESP_ODOMETER:

      fprintf (stderr, "Odometer: %04x %04x\n", resp->u.odometer.motor0_odometer, resp->u.odometer.motor1_odometer);

      break;

    default:

      fprintf (stderr, "[Unknown]\n");
    }
}

int
main (int argc, char **argv)
{
  int fd;
  struct motor_request req;
  struct termios tty;

  if (-1 == (fd = open(TTY, O_RDWR | O_NOCTTY)))
    err(EXIT_FAILURE, "Failed to open '%s' in read/write mode", TTY);

  if (-1 == tcflush(fd, TCIFLUSH))
    err(EXIT_FAILURE, "tcflush failed");

  memset (&tty, 0, sizeof tty);

  if (-1 == tcgetattr (fd, &tty))
    err(EXIT_FAILURE, "tcgetattr failed");

  cfsetospeed (&tty, 115200);
  cfsetispeed (&tty, 115200);
  cfmakeraw (&tty);
  tty.c_cflag |= CLOCAL | CREAD | CS8;
  tty.c_iflag |= ICRNL;

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
    err(EXIT_FAILURE, "tcsetattr failed");

  req.sync = 0xff;
  req.type = MOTOR_REQ_HELLO;
  req.u.hello.magic_a = (MOTOR_MAGIC_A);
  req.u.hello.magic_b = (MOTOR_MAGIC_B);

  /* Write twice to force byte stream synchronization.  */
  write_all (fd, &req, sizeof(req));
  write_all (fd, &req, sizeof(req));

  for (;;)
    {
      unsigned char buffer[sizeof(struct motor_response)];
      size_t fill = 0;

      while (fill < sizeof (buffer))
        {
          if (1 != read(fd, &buffer[fill++], 1))
            err(EXIT_FAILURE, "Read error");

          if (buffer[0] != 0xff)
            fill = 0;
        }

      process_response((struct motor_response *) buffer);

      fill = 0;
    }

  return EXIT_SUCCESS;
}
