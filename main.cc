#include "hardware.h"
#include "motor.h"
#include "serial.h"

static struct motor motors[2];
static int expecting_hello, expecting_error;

static void
process_request(unsigned char ch);

static void
generate_response(void *buffer, uint16_t *size);

static unsigned char rx_buffer[sizeof(struct motor_request)] __attribute__((aligned(4)));
static unsigned int rx_fill;

int
main()
{
  init();

  serial_open(115200, process_request, generate_response);

  /* Set PWM frequency to 31.25 kHz.  */
  TCCR1B &= 0xf8 | 0x01;

  motors[0].set_pwm_pin(9);
  motors[0].set_sensor_pins(A0, A1, A2);
  motors[0].set_output_pins(2, 3, 4);

  motors[1].set_pwm_pin(10);
  motors[1].set_sensor_pins(A5, A4, A3);
  motors[1].set_output_pins(5, 6, 7);

  for (;;)
    {
      motors[0].update();
      motors[1].update();
    }
}

static void
process_request(unsigned char ch)
{
  rx_buffer[rx_fill++] = ch;

  if (rx_buffer[0] == MOTOR_SYNC_BYTE)
    {
      rx_fill = 0;

      return;
    }

  if (rx_fill != sizeof(rx_buffer))
    return;

  struct motor_request *request
    = reinterpret_cast<struct motor_request*>(rx_buffer);

  switch (request->type)
    {
    case MOTOR_REQ_HELLO:

      if (request->u.hello.magic_a != MOTOR_MAGIC_A ||
          request->u.hello.magic_b != MOTOR_MAGIC_B)
        {
          expecting_error = 2;

          break;
        }

      expecting_hello = 1;

      break;

    case MOTOR_REQ_POWER:

      motors[0].set_power(request->u.power.motor0_power);
      motors[1].set_power(request->u.power.motor1_power);

      break;

    default:

      expecting_error = 1;
    }

  rx_fill = 0;
}

static void
generate_response(void *buffer, uint16_t *size)
{
  struct motor_response* tx_buffer
    = reinterpret_cast<struct motor_response*>(buffer);
  *size = sizeof(*tx_buffer);

  tx_buffer->sync = MOTOR_SYNC_BYTE;

  if (expecting_hello)
    {
      tx_buffer->type = MOTOR_RESP_HELLO;
      tx_buffer->u.hello.magic_a = MOTOR_MAGIC_A;
      tx_buffer->u.hello.magic_b = MOTOR_MAGIC_B;
      motors[0].reset();
      motors[1].reset();
      expecting_hello = 0;
    }
  else if (expecting_error)
    {
      tx_buffer->type = MOTOR_RESP_ERROR;
      tx_buffer->u.error.reserved0 = 0;
      tx_buffer->u.error.reserved1 = 0;
      expecting_error = 0;
    }
  else
    {
      tx_buffer->type = MOTOR_RESP_ODOMETER;
      tx_buffer->u.odometer.motor0_odometer = motors[0].odometer();
      tx_buffer->u.odometer.motor1_odometer = motors[1].odometer();
    }
}

extern "C" void
exit(int status)
{
  for (;;)
    ;
}
