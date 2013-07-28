#define OPT_BOARD_INTERNAL
#include "mpide/hardware/pic32/cores/pic32/System_Defs.h"
#include "mpide/hardware/pic32/variants/Uno32/Board_Defs.h"
#include "mpide/hardware/pic32/cores/pic32/wiring.h"

#include "serial.h"

#define SENSOR_MOTOR0_A A3
#define SENSOR_MOTOR0_B A4
#define SENSOR_MOTOR0_C A5

int
main()
{
  int signal;

  init();

  serial_init();
  serial_open(115200);

  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);

  pinMode(SENSOR_MOTOR0_A, INPUT);
  pinMode(SENSOR_MOTOR0_B, INPUT);
  pinMode(SENSOR_MOTOR0_C, INPUT);

  for (;;)
    {
      serial_write('H');
      serial_write('E');
      serial_write('L');
      serial_write('O');
      serial_write(' ');

      /* Health indicator */
      digitalWrite(PIN_LED1, (millis() >> 8) & 1);
      digitalWrite(PIN_LED2, (millis() >> 9) & 1);
    }
}

extern "C" void
exit(int status)
{
  for (;;)
    ;
}
