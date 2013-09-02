#include "hardware.h"
#include "motor.h"

/* HALL_SENSOR_THRESHOLD is specified in multiples of of 4.9 mV. */
#define HALL_SENSOR_THRESHOLD 510  /* 2.5 V */

motor::motor()
{
  reset();
}

void
motor::set_pwm_pin(int pin)
{
  analogWrite(pwm_ = pin, 0);
  pinMode(pwm_, OUTPUT);
}

void
motor::set_output_pins(int a, int b, int c)
{
  digitalWrite(output_a_ = a, 0);
  digitalWrite(output_b_ = b, 0);
  digitalWrite(output_c_ = c, 0);
  pinMode(output_a_, OUTPUT);
  pinMode(output_b_, OUTPUT);
  pinMode(output_c_, OUTPUT);
}

void
motor::set_sensor_pins(int a, int b, int c)
{
  pinMode(sensor_a_ = a, INPUT);
  pinMode(sensor_b_ = b, INPUT);
  pinMode(sensor_c_ = c, INPUT);
}

void
motor::update()
{
  int new_orientation;

  new_orientation = read_orientation();

  if (new_orientation == orientation_)
    return;

  if (new_orientation != -1)
    {
      if (orientation_ != -1)
        {
          if (orientation_ + 1 == new_orientation ||
              orientation_ == new_orientation - 1)
            ++odometer_;
          else if (new_orientation + 1 == orientation_ ||
                   new_orientation == orientation_ - 1)
            --odometer_;
        }

      orientation_ = new_orientation;
      orientation_valid_ = 1;
    }
  else
    orientation_valid_ = 0;

  commutate(new_orientation);
}

void
motor::reset()
{
  orientation_ = -1;
  odometer_ = 0;
  power_ = 0;
}

int
motor::read_orientation()
{
  /* Hall sensors:  AA   A
    *                BBB
    *                  CCC
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

  sensor_a_state = analogRead(sensor_a_) > HALL_SENSOR_THRESHOLD;
  sensor_b_state = analogRead(sensor_b_) > HALL_SENSOR_THRESHOLD;
  sensor_c_state = analogRead(sensor_c_) > HALL_SENSOR_THRESHOLD;

  return orientation_map[(sensor_c_state << 2) | (sensor_b_state << 1) | sensor_a_state];
}

void
motor::commutate(int orientation)
{
  if (power_ == 0)
    {
      analogWrite(pwm_, 0);
      digitalWrite(output_a_, 0); pinMode(output_a_, OUTPUT);
      digitalWrite(output_b_, 0); pinMode(output_b_, OUTPUT);
      digitalWrite(output_c_, 0); pinMode(output_c_, OUTPUT);
    }
  else if (orientation == -1)
    {
      analogWrite(pwm_, 0);
      pinMode(output_a_, INPUT);
      pinMode(output_b_, INPUT);
      pinMode(output_c_, INPUT);
    }
  else
    {
      static const signed char force[6][3] =
        {
            {  1, -1,  0 },
            {  1,  0, -1 },
            {  0,  1, -1 },
            { -1,  1,  0 },
            { -1,  0,  1 },
            {  0, -1,  1 },
        };
      signed char force_a, force_b, force_c;
      unsigned char effective_power;

      force_a = force[orientation][0];
      force_b = force[orientation][1];
      force_c = force[orientation][2];

      if (power_ > 0)
        effective_power = power_;
      else
        {
          force_a = -force_a;
          force_b = -force_b;
          force_c = -force_c;
          effective_power = -power_;
        }

      analogWrite(pwm_, effective_power);

#define HANDLE_PIN(OUTPUT_PIN, VALUE) \
      switch (VALUE) \
        { \
        case -1: \
          digitalWrite(OUTPUT_PIN, 0); \
          pinMode(OUTPUT_PIN, OUTPUT); \
          break; \
        case 0: \
          pinMode(OUTPUT_PIN, INPUT); \
          break; \
        case 1: \
          digitalWrite(OUTPUT_PIN, 1); \
          pinMode(OUTPUT_PIN, OUTPUT); \
          break; \
        }

      HANDLE_PIN(output_a_, force_a)
      HANDLE_PIN(output_b_, force_b)
      HANDLE_PIN(output_c_, force_c)
#undef HANDLE_PIN
    }
}
