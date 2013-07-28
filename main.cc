#include "mpide/hardware/pic32/cores/pic32/System_Defs.h"
#include "mpide/hardware/pic32/variants/Uno32/Board_Defs.h"
#include "mpide/hardware/pic32/cores/pic32/wiring.h"

#include "motor.h"
#include "serial.h"

#define SENSOR_MOTOR0_A A3
#define SENSOR_MOTOR0_B A4
#define SENSOR_MOTOR0_C A5

struct motor
{
  motor()
    : odometer(0), power(0)
  {
  }

  unsigned int odometer;
  signed short power;
};

static struct motor motors[2];
static int expecting_hello;

int
main()
{
  int signal;

  init();

  serial_open(115200);

  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);

  pinMode(SENSOR_MOTOR0_A, INPUT);
  pinMode(SENSOR_MOTOR0_B, INPUT);
  pinMode(SENSOR_MOTOR0_C, INPUT);

  for (;;)
    {
      /* Health indicator */
      digitalWrite(PIN_LED1, (millis() >> 9) & 1);
    }
}

void motor_process_request(const struct motor_request* rx_buffer)
{
  switch (rx_buffer->type)
    {
    case MOTOR_REQ_HELLO:

      if (rx_buffer->u.hello.magic_a != MOTOR_MAGIC_A ||
          rx_buffer->u.hello.magic_b != MOTOR_MAGIC_B)
        break;

      expecting_hello = 1;

      break;

    case MOTOR_REQ_POWER:

      motors[0].power = rx_buffer->u.power.motor0_power;
      motors[1].power = rx_buffer->u.power.motor1_power;

      break;
    }
}

void motor_generate_response(struct motor_response* tx_buffer)
{
  if (expecting_hello)
    {
      tx_buffer->sync = 0xff;
      tx_buffer->type = MOTOR_RESP_HELLO;
      tx_buffer->u.hello.magic_a = MOTOR_MAGIC_A;
      tx_buffer->u.hello.magic_b = MOTOR_MAGIC_B;
      motors[0].odometer = 0;
      motors[1].odometer = 0;
    }
  else
    {
      tx_buffer->sync = 0xff;
      tx_buffer->type = MOTOR_RESP_ODOMETER;
      tx_buffer->u.odometer.motor0_odometer = motors[0].odometer;
      tx_buffer->u.odometer.motor1_odometer = motors[1].odometer;
    }
}

extern "C" void
exit(int status)
{
  for (;;)
    ;
}
