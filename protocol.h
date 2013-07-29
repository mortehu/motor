#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define MOTOR_MAGIC_A 0xd4bb
#define MOTOR_MAGIC_B 0xeed5

enum motor_request_id
{
  MOTOR_REQ_HELLO = 0x00,
  MOTOR_REQ_POWER = 0x01,
};

enum motor_response_id
{
  MOTOR_RESP_HELLO =    0x00,
  MOTOR_RESP_ERROR =    0x01,
  MOTOR_RESP_ODOMETER = 0x02,
};

struct motor_request
{
  unsigned char sync; /* 0xff */
  unsigned char type;

  union
    {
      struct
        {
          /* MOTOR_MAGIC_A */
          unsigned short magic_a;

          /* MOTOR_MAGIC_B */
          unsigned short magic_b;
        } hello;

      struct
        {
          /* These values denote the amount of power that should be delivered to each
           * motor.
           *
           * The range  32767 to  1 indicates forward power.
           * The range -32767 to -1 indicates reverse power.
           * The value 0 indicates that the motor coils should be grounded so that the
           *   torsion resistance is high.  This mode does not draw any current from
           *   the motor power supply.
           * The value -32768 denotes that current should flow freely between coils, so
           *   that the torsion resistance is low.  This mode does not draw any current
           *   from the motor power supply.
           */
          signed short motor0_power;
          signed short motor1_power;
        } power;
    } u;
};

struct motor_response
{
  unsigned char sync; /* 0xff */
  unsigned char type;

  union
    {
      struct
        {
          /* MOTOR_MAGIC_A */
          unsigned short magic_a;

          /* MOTOR_MAGIC_B */
          unsigned short magic_b;
        } hello;

      struct
        {
          unsigned short reserved0;
          unsigned short reserved1;
        } error;

      struct
        {
          /* These values denote the total distance traveled in units of Hall effect
           * sensor transitions.
           */
          unsigned short motor0_odometer;
          unsigned short motor1_odometer;
        } odometer;
    } u;
};

#endif /* !PROTOCOL_H_ */
