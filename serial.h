#ifndef SERIAL_H_
#define SERIAL_H_ 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MESSAGE_SIZE 128

enum serial_baud_rate
{
  SERIAL_57600,
  SERIAL_115200,
};

typedef void (*rx_callback)(unsigned char ch);
typedef void (*tx_callback)(void *buffer, uint16_t *size);

void
serial_open(enum serial_baud_rate baud_rate, rx_callback rx, tx_callback tx);

void
serial_close(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* !SERIAL_H_ */
