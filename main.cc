#include "hardware.h"

int
main()
{
  init();

  /* PWM */
  analogWrite(9,  32); pinMode(9, OUTPUT);
  analogWrite(10, 32); pinMode(10, OUTPUT);

  for (int i = 2; i <= 7; ++i)
    pinMode(i, INPUT);

  for (;;)
    {
      for (int i = 2; i <= 7; ++i)
        {
          pinMode(i, OUTPUT);
          digitalWrite(i, 1);

          delay(100);
          pinMode(i, INPUT);
        }
    }
}

extern "C" void
exit(int status)
{
  for (;;)
    ;
}
