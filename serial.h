#ifndef SERIAL_H_
#define SERIAL_H_ 1

#include <stdint.h>

#include "protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

void
serial_open(uint32_t baud_rate);

void
serial_close(void);

void
motor_process_request(const struct motor_request* rx_buffer);

void
motor_generate_response(struct motor_response* tx_buffer);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* !SERIAL_H_ */
