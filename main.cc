#include "mpide/hardware/pic32/variants/Uno32/Board_Defs.h"
#include "mpide/hardware/pic32/cores/pic32/wiring.h"
#include "mpide/hardware/pic32/cores/pic32/HardwareSerial.h"
#include "mpide/hardware/pic32/libraries/Wire/Wire.h"

#define SENSOR_MOTOR0_A A3
#define SENSOR_MOTOR0_B A4
#define SENSOR_MOTOR0_C A5

int
main()
{
  int signal, count = 0;

  init();

  Wire.begin();
  /* XXX(mortehu): Not working Serial.begin(9600); */

  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);

  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);

  pinMode(SENSOR_MOTOR0_A, INPUT);
  pinMode(SENSOR_MOTOR0_B, INPUT);
  pinMode(SENSOR_MOTOR0_C, INPUT);

  for (;;)
    {
      signal = analogRead(SENSOR_MOTOR0_A);
      signal >>= 7;

      /* Health indicator */
      digitalWrite(PIN_LED1, (millis() >> 8) & 1);
      digitalWrite(PIN_LED2, (millis() >> 9) & 1);

      /* 3 bit analog readout */
      digitalWrite(0, analogRead(SENSOR_MOTOR0_A) > 128);
      digitalWrite(1, analogRead(SENSOR_MOTOR0_B) > 128);
      digitalWrite(2, analogRead(SENSOR_MOTOR0_C) > 128);
    }
}

extern "C" void
exit(int status)
{
  for (;;)
    ;
}
