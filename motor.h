#ifndef MOTOR_H_
#define MOTOR_H_ 1

#include "protocol.h"

class motor
{
public:
  motor();

  void set_sensor_pins(int a, int b, int c);

  int read_orientation();

  void update();

  void reset();

  unsigned int odometer() const { return odometer_; }

  void set_power(signed short power) { power_ = power; }

private:
  unsigned int odometer_;
  int orientation_;

  int sensor_a_, sensor_b_, sensor_c_;
  signed short power_;
};

#endif /* !MOTOR_H_ */
