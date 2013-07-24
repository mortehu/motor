#include "mpide/hardware/pic32/cores/pic32/wiring.h"
#include "mpide/hardware/pic32/variants/Uno32/Board_Defs.h"

int
main()
{
  init();

  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);

  for (;;)
    {
      digitalWrite(PIN_LED1, LOW);
      digitalWrite(PIN_LED2, HIGH);
      delay(300);

      digitalWrite(PIN_LED1, HIGH);
      digitalWrite(PIN_LED2, LOW);
      delay(300);
    }
}

extern "C" void
exit(int status)
{
  for (;;)
    ;
}
