#ifndef MOTOR_H_
#define MOTOR_H_ 1

#include "protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

void motor_process_request(const struct motor_request* rx_buffer);
void motor_generate_response(struct motor_response* tx_buffer);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* !MOTOR_H_ */
