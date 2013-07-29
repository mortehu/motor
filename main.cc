#include "mpide/hardware/pic32/cores/pic32/System_Defs.h"
#include "mpide/hardware/pic32/variants/Uno32/Board_Defs.h"
#include "mpide/hardware/pic32/cores/pic32/wiring.h"

#include "motor.h"
#include "serial.h"

/* HALL_SENSOR_THRESHOLD is specified in multiples of of 4.9 mV. */
#define HALL_SENSOR_THRESHOLD 10 /* 49 mV */

struct motor
{
  motor()
    : odometer(0), power(0)
  {
  }

  void set_sensor_pins(int a, int b, int c)
  {
    pinMode(sensor_a = a, INPUT);
    pinMode(sensor_b = b, INPUT);
    pinMode(sensor_c = c, INPUT);
  }

  int read_orientation()
  {
    /* Hall sensors:  AA   A
     *                 BBB
     *                   CCC
     *
     * Orientation:   Magnet:
     * 0              *
     * 1               *
     * 2                *
     * 3                 *
     * 4                  *
     * 5                   *
     */
    static const int orientation_map[8] =
      {
        -1, /*     */
         0, /*   A */
         2, /*  B  */
         1, /*  BA */
         4, /* C   */
         5, /* C A */
         3, /* CB  */
        -1, /* CBA */
      };

    unsigned int sensor_a_state, sensor_b_state, sensor_c_state;

    sensor_a_state = analogRead(sensor_a) > HALL_SENSOR_THRESHOLD;
    sensor_b_state = analogRead(sensor_b) > HALL_SENSOR_THRESHOLD;
    sensor_c_state = analogRead(sensor_c) > HALL_SENSOR_THRESHOLD;

    return orientation_map[(sensor_c_state << 2) | (sensor_b_state << 1) | sensor_a_state];
  }

  void update()
  {
    int new_orientation;

    new_orientation = read_orientation();

    if (new_orientation == orientation)
      return;

    if (new_orientation == -1)
      return;

    if (orientation != -1)
      {
        if ((orientation + 1) % 6 == new_orientation)
          ++odometer;
        else if ((new_orientation + 1) % 6 == orientation)
          --odometer;
      }

    orientation = new_orientation;
  }

  unsigned int odometer;
  int orientation;

  int sensor_a, sensor_b, sensor_c;
  signed short power;
};

static struct motor motors[2];
static int expecting_hello, expecting_error;

int
main()
{
  init();

  serial_open(115200);

  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);

  motors[0].set_sensor_pins(A3, A4, A5);
  motors[1].set_sensor_pins(A0, A1, A2);

  for (;;)
    {
      /* Health indicator */
      digitalWrite(PIN_LED1, (millis() >> 9) & 1);

      motors[0].update();
      motors[1].update();
    }
}

void motor_process_request(const struct motor_request* rx_buffer)
{
  switch (rx_buffer->type)
    {
    case MOTOR_REQ_HELLO:

      if (rx_buffer->u.hello.magic_a != MOTOR_MAGIC_A ||
          rx_buffer->u.hello.magic_b != MOTOR_MAGIC_B)
        {
          expecting_error = 2;

          break;
        }

      expecting_hello = 1;

      break;

    case MOTOR_REQ_POWER:

      motors[0].power = rx_buffer->u.power.motor0_power;
      motors[1].power = rx_buffer->u.power.motor1_power;

      break;

    default:

      expecting_error = 1;
    }
}

void motor_generate_response(struct motor_response* tx_buffer)
{
  tx_buffer->sync = 0xff;

  if (expecting_hello)
    {
      tx_buffer->type = MOTOR_RESP_HELLO;
      tx_buffer->u.hello.magic_a = MOTOR_MAGIC_A;
      tx_buffer->u.hello.magic_b = MOTOR_MAGIC_B;
      motors[0].odometer = 0;
      motors[1].odometer = 0;
      expecting_hello = 0;
    }
  else if (expecting_error)
    {
      tx_buffer->type = MOTOR_RESP_ERROR;
      tx_buffer->u.error.reserved0 = expecting_error;
      tx_buffer->u.error.reserved1 = 0;
      expecting_error = 0;
    }
  else
    {
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
