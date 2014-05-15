#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>

#define MOTOR_SYNC_BYTE0 0xff
#define MOTOR_SYNC_BYTE1 0xfe

enum motor_message_id
{
  MOTOR_MSG_REQUEST_SPEED = 0x01,
  MOTOR_MSG_REQUEST_ACCELERATION = 0x02,
  MOTOR_MSG_ODOMETER = 0x03,
  MOTOR_MSG_FET_MASK = 0x04,
  MOTOR_MSG_REQUEST_VARS = 0x05,
  MOTOR_MSG_VAR = 0x06,
};

enum var_id
{
#define VAR_DEFINE(symbol, description) symbol,
#include "variables.h"
#undef VAR_DEFINE

  VAR_LAST,
};

struct motor_message
{
  unsigned char sync[2]; /* MOTOR_SYNC_BYTE0, MOTOR_SYNC_BYTE1 */
  unsigned char crc8; /* CRC-8 of `type' and remaining bytes */
  unsigned char type;

  union
    {
      struct
        {
          /* Sets the target speed of both motors in Hall effect sensor
           * transitions per second.  */
          int16_t motor0_speed;
          int16_t motor1_speed;
        } speed;

      struct
        {
          /* Sets the target acceleration of both motors in Hall effect sensor
           * transitions per second per second.  */
          int16_t motor0_max_acceleration;
          int16_t motor1_max_acceleration;
        } acceleration;

      struct
        {
          /* These values denote the total distance traveled in units of Hall
           * effect sensor transitions.  */
          uint16_t motor0_odometer;
          uint16_t motor1_odometer;
        } odometer;

      struct
        {
          uint8_t id;
          uint8_t reserved;
          int16_t value;
        } var;
    } u;
};

#endif /* !PROTOCOL_H_ */
