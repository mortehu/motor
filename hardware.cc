#define ARDUINO_MAIN
#include "hardware.h"

#if defined(SYSTEM_ARDUINO)
#  include "wiring.c"
#  include "wiring_analog.c"
#  include "wiring_digital.c"
#elif defined(SYSTEM_UNO32)
#  include "pins_arduino.c"
#  include "task_manager.c"
#  include "wiring.c"
#  include "wiring_analog.c"
#  include "wiring_digital.c"
#else
void
init(void)
{
}

void
pinMode(uint8_t pin, enum pin_mode_t mode)
{
}

void
digitalWrite(uint8_t pin, uint8_t value)
{
}

int
analogRead(uint8_t pin)
{
  return 0;
}

void
analogWrite(uint8_t pin, uint8_t value)
{
}
#endif
