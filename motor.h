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

  void update(uint32_t time);

  void reset();

  unsigned int odometer() const { return odometer_; }
  int8_t orientation() const { return orientation_; }
  signed short power() const { return power_; }

  void set_speed(int16_t speed) { target_speed_ = speed; }

  void quick_stop(void)
  {
    target_speed_ = 0;
    speed_ = 0;
    power_ = 0;
    previous_error_ = 0;
    integral_ = 0;
    commutate();
  }

  /* Call every 10 milliseconds.  */
  void pid_update();

private:
  static const int8_t invalid_orientation;

  void set_power(signed short power) { power_ = power; commutate(); }

  int8_t read_orientation();

  void commutate();

  uint16_t odometer_;
  int8_t orientation_;

  uint8_t pwm_;
  uint8_t sensor_a_, sensor_b_, sensor_c_;
  uint8_t output_a_, output_b_, output_c_;
  int16_t power_;

  /* PID control manipulated variables.  */
  int32_t target_speed_;

  /* PID control process variables.  */
  int32_t speed_;
  uint32_t last_time_;

  /* PID control terms.  */
  int32_t previous_error_;
  int32_t integral_;
};

#endif /* !MOTOR_H_ */
