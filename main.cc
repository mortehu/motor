#include "crc8.h"
#include "hardware.h"
#include "motor.h"
#include "serial.h"

static struct motor motors[2];
static signed short motor0_max_acceleration, motor1_max_acceleration;

/* The time at which we will stop the motors, by setting the target speed for
 * the PID controller to zero.  */
static unsigned long motor_brake_timeout;

/* The time at which we will freeze the motors, by grounding the wires.  */
static unsigned long motor_freeze_timeout;

/* Process one byte of data from the serial bus.  */
static void process_request(unsigned char ch);

static void generate_message(void *buffer, uint16_t *size);

static unsigned char rx_buffer[sizeof(struct motor_message)];
static unsigned int rx_fill;

static bool vars_requested;
static uint8_t var_progress;

static uint32_t next_pid_update;

int main() {
  init();

  serial_open(SERIAL_115200, process_request, generate_message);

#if defined(HERCULES3_1)
  /* Set PWM frequency to 16.125 kHz.  */
  TCCR3B &= 0xf8 | 0x01;
  TCCR4B &= 0xf8 | 0x01;
#elif defined(SYSTEM_ARDUINO)
  /* Set PWM frequency to 31.25 kHz.  */
  TCCR1B &= 0xf8 | 0x01;
#endif

  motors[0].set_pwm_pins(5, 2, 3);
  motors[0].set_sensor_pins(A2, A1, A0);
  motors[0].set_output_pins(22, 23, 24);

  motors[1].set_pwm_pins(6, 7, 8);
  motors[1].set_sensor_pins(A3, A4, A5);
  motors[1].set_output_pins(25, 26, 27);

  for (;;) {
    uint32_t now = micros();

    if (now > motor_freeze_timeout) {
      /* Instantly grounds the power wires.  This is a safeguard against
       * bugs in the PID controller.  */
      motors[0].quick_stop();
      motors[1].quick_stop();
    } else if (now > motor_brake_timeout) {
      /* Smoothly stops the motors.  */
      motors[0].set_speed(0);
      motors[1].set_speed(0);
    }

    if (now >= next_pid_update) {
      motors[0].pid_update();
      motors[1].pid_update();

      next_pid_update = now + 10000;
    }

    motors[0].update(now);
    motors[1].update(now);
  }
}

static void process_request(unsigned char ch) {
  rx_buffer[rx_fill++] = ch;

  if (rx_fill == 1) {
    if (rx_buffer[0] != MOTOR_SYNC_BYTE0) rx_fill = 0;
  } else if (rx_fill == 2) {
    if (rx_buffer[1] != MOTOR_SYNC_BYTE1) rx_fill = 0;
  } else if (rx_fill == sizeof(rx_buffer)) {
    struct motor_message *request =
        reinterpret_cast<struct motor_message *>(rx_buffer);

    if (request->crc8 != crc8(&request->type, sizeof(*request) - 3)) {
      rx_fill = 0;

      return;
    }

    switch (request->type) {
      case MOTOR_MSG_REQUEST_SPEED:

        motor_brake_timeout = micros() + 500000;
        motor_freeze_timeout = micros() + 1000000;
        motors[0].set_speed(request->u.speed.motor0_speed);
        motors[1].set_speed(-request->u.speed.motor1_speed);

        break;

      case MOTOR_MSG_REQUEST_ACCELERATION:

        motor0_max_acceleration =
            request->u.acceleration.motor0_max_acceleration;
        motor1_max_acceleration =
            request->u.acceleration.motor1_max_acceleration;

        break;

      case MOTOR_MSG_REQUEST_VARS:

        vars_requested = true;
        var_progress = 0;

        break;
    }

    rx_fill = 0;
  }
}

static void generate_message(void *buffer, uint16_t *size) {
  struct motor_message *message =
      reinterpret_cast<struct motor_message *>(buffer);
  *size = sizeof(*message);

  message->sync[0] = MOTOR_SYNC_BYTE0;
  message->sync[1] = MOTOR_SYNC_BYTE1;

  if (vars_requested) {
    message->type = MOTOR_MSG_VAR;
    message->u.var.id = var_progress++;
    if (message->u.var.id >= VAR_LAST) vars_requested = false;

    switch (message->u.var.id) {
      case VAR_MOTOR0_INVALID_TRANSITIONS:
        message->u.var.value = motors[0].invalid_transitions();
        break;
      case VAR_MOTOR0_INVALID_STATES:
        message->u.var.value = motors[0].invalid_states();
        break;
      case VAR_MOTOR1_INVALID_TRANSITIONS:
        message->u.var.value = motors[1].invalid_transitions();
        break;
      case VAR_MOTOR1_INVALID_STATES:
        message->u.var.value = motors[1].invalid_states();
        break;
      default:
        message->u.var.value = 0xdead;
    }
  } else {
    message->type = MOTOR_MSG_ODOMETER;
    message->u.odometer.motor0_odometer = motors[0].odometer();
    message->u.odometer.motor1_odometer = motors[1].odometer();
  }

  message->crc8 = crc8(&message->type, sizeof(*message) - 3);
}

extern "C" void exit(int status) {
  for (;;)
    ;
}
