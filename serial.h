#ifndef SERIAL_H_
#define SERIAL_H_ 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MESSAGE_SIZE 128

typedef void (*rx_callback)(unsigned char ch);
typedef void (*tx_callback)(void *buffer, uint16_t *size);

void
serial_open(uint32_t baud_rate, rx_callback rx, tx_callback tx);

void
serial_close(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* !SERIAL_H_ */
