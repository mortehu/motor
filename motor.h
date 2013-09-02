#ifndef MOTOR_H_
#define MOTOR_H_ 1

#include "protocol.h"

class motor
{
public:
  motor();

  void set_pwm_pin(int pin);
  void set_sensor_pins(int a, int b, int c);
  void set_output_pins(int a, int b, int c);

  void update();

  void reset();

  unsigned int odometer() const { return odometer_; }
  int orientation() const { return orientation_; }
  int orientation_valid() const { return orientation_valid_; }
  signed short power() const { return power_; }

  void set_power(signed short power) { power_ = power; commutate(orientation_); }

private:
  int read_orientation();

  void commutate(int orientation);

  unsigned int odometer_;
  int orientation_;
  int orientation_valid_;

  int pwm_;
  int sensor_a_, sensor_b_, sensor_c_;
  int output_a_, output_b_, output_c_;
  signed short power_;
};

#endif /* !MOTOR_H_ */
