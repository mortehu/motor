#ifndef HARDWARE_H_
#define HARDWARE_H_ 1

#if defined(SYSTEM_ARDUINO)
#  include <Arduino.h>
#elif defined(SYSTEM_UNO32)
#  include <System_Defs.h>
#  include <Board_Defs.h>
#  include <wiring.h>
#elif defined(SYSTEM_GNU)
#  include <stdint.h>

#  ifdef __cplusplus
extern "C" {
#  endif

enum pin_mode_t
{
  INPUT,
  OUTPUT
};

enum pin_t
{
  A0,
  A1,
  A2,
  A3,
  A4,
  A5,
};

void
init(void);

void
pinMode(uint8_t pin, enum pin_mode_t mode);

void
digitalWrite(uint8_t pin, uint8_t value);

int
analogRead(uint8_t pin);

void
analogWrite(uint8_t pin, uint8_t value);

#  ifdef __cplusplus
} /* extern "C" */
#  endif
#endif

#endif /* !HARDWARE_H_ 1 */
