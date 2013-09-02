#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define MOTOR_SYNC_BYTE0 0xff
#define MOTOR_SYNC_BYTE1 0xfe

enum motor_message_id
{
  MOTOR_MSG_REQUEST_SPEED = 0x01,
  MOTOR_MSG_REQUEST_ACCELERATION = 0x02,
  MOTOR_MSG_ODOMETER = 0x03,
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
          signed short motor0_speed;
          signed short motor1_speed;
        } speed;

      struct
        {
          /* Sets the target acceleration of both motors in Hall effect sensor
           * transitions per second per second.  */
          signed short motor0_max_acceleration;
          signed short motor1_max_acceleration;
        } acceleration;

      struct
        {
          /* These values denote the total distance traveled in units of Hall
           * effect sensor transitions.  */
          unsigned short motor0_odometer;
          unsigned short motor1_odometer;
        } odometer;
    } u;
};

#endif /* !PROTOCOL_H_ */
