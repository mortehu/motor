#include "crc8.h"
#include "hardware.h"
#include "motor.h"
#include "serial.h"

static struct motor motors[2];
static signed short motor0_requested_speed, motor1_requested_speed;
static signed short motor0_max_acceleration, motor1_max_acceleration;

static void
process_request(unsigned char ch);

static void
generate_message(void *buffer, uint16_t *size);

static unsigned char rx_buffer[sizeof(struct motor_message)];
static unsigned int rx_fill;

int
main()
{
  init();

  serial_open(SERIAL_115200, process_request, generate_message);

#if defined(SYSTEM_ARDUINO)
  /* Set PWM frequency to 31.25 kHz.  */
  TCCR1B &= 0xf8 | 0x01;
#endif

  motors[0].set_pwm_pin(9);
  motors[0].set_sensor_pins(A0, A1, A2);
  motors[0].set_output_pins(2, 3, 4);

  motors[1].set_pwm_pin(10);
  motors[1].set_sensor_pins(A3, A4, A5);
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

  if (rx_fill == 1)
    {
      if (rx_buffer[0] != MOTOR_SYNC_BYTE0)
        rx_fill = 0;
    }
  else if (rx_fill == 2)
    {
      if (rx_buffer[1] != MOTOR_SYNC_BYTE1)
        rx_fill = 0;
    }
  else if (rx_fill == sizeof(rx_buffer))
    {
      struct motor_message *request
        = reinterpret_cast<struct motor_message*>(rx_buffer);

      if (request->crc8 != crc8(&request->type, sizeof(*request) - 3))
        {
          rx_fill = 0;

          return;
        }

      switch (request->type)
        {
        case MOTOR_MSG_REQUEST_SPEED:

          motor0_requested_speed = request->u.speed.motor0_speed;
          motor1_requested_speed = request->u.speed.motor1_speed;

          motors[0].set_power(request->u.speed.motor0_speed);
          motors[1].set_power(-request->u.speed.motor1_speed);

          break;

        case MOTOR_MSG_REQUEST_ACCELERATION:

          motor0_max_acceleration = request->u.acceleration.motor0_max_acceleration;
          motor1_max_acceleration = request->u.acceleration.motor1_max_acceleration;

          break;
        }

      rx_fill = 0;
    }
}

static void
generate_message(void *buffer, uint16_t *size)
{
  struct motor_message* message
    = reinterpret_cast<struct motor_message*>(buffer);
  *size = sizeof(*message);

  message->sync[0] = MOTOR_SYNC_BYTE0;
  message->sync[1] = MOTOR_SYNC_BYTE1;
  message->type = MOTOR_MSG_ODOMETER;
  message->u.odometer.motor0_odometer = motors[0].odometer();
  message->u.odometer.motor1_odometer = motors[1].odometer();
  message->crc8 = crc8(&message->type, sizeof(*message) - 3);
}

extern "C" void
exit(int status)
{
  for (;;)
    ;
}
