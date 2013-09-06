#ifndef MOTOR_H_
#define MOTOR_H_ 1

#include <stdint.h>

#include "protocol.h"

class motor
{
public:
  motor();

  void set_pwm_pin(uint8_t pin);
  void set_sensor_pins(uint8_t a, uint8_t b, uint8_t c);
  void set_output_pins(uint8_t a, uint8_t b, uint8_t c);

  void update();

  void reset();

  unsigned int odometer() const { return odometer_; }
  int8_t orientation() const { return orientation_; }
  signed short power() const { return power_; }

  void set_power(signed short power) { power_ = power; commutate(); }

private:
  int8_t read_orientation();

  void commutate();

  uint16_t odometer_;
  int8_t orientation_;

  uint8_t pwm_;
  uint8_t sensor_a_, sensor_b_, sensor_c_;
  uint8_t output_a_, output_b_, output_c_;
  int16_t power_;
};

#endif /* !MOTOR_H_ */
