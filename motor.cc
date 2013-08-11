#include "hardware.h"
#include "motor.h"

/* HALL_SENSOR_THRESHOLD is specified in multiples of of 4.9 mV. */
#define HALL_SENSOR_THRESHOLD 10 /* 49 mV */

motor::motor()
{
  reset();
}

void
motor::set_sensor_pins(int a, int b, int c)
{
  pinMode(sensor_a_ = a, INPUT);
  pinMode(sensor_b_ = b, INPUT);
  pinMode(sensor_c_ = c, INPUT);
}

int
motor::read_orientation()
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

  sensor_a_state = analogRead(sensor_a_) > HALL_SENSOR_THRESHOLD;
  sensor_b_state = analogRead(sensor_b_) > HALL_SENSOR_THRESHOLD;
  sensor_c_state = analogRead(sensor_c_) > HALL_SENSOR_THRESHOLD;

  return orientation_map[(sensor_c_state << 2) | (sensor_b_state << 1) | sensor_a_state];
}

void
motor::update()
{
  int new_orientation;

  new_orientation = read_orientation();

  if (new_orientation == orientation_)
    return;

  if (new_orientation == -1)
    return;

  if (orientation_ != -1)
    {
      if ((orientation_ + 1) % 6 == new_orientation)
        ++odometer_;
      else if ((new_orientation + 1) % 6 == orientation_)
        --odometer_;
    }

  orientation_ = new_orientation;
}

void
motor::reset()
{
  odometer_ = 0;
  power_ = 0;
}
