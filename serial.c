/* Specialized serial interface for motor controller.
 *
 * References:
 *
 *  1. PIC32MX3XX/4XX Datasheet, Preliminary, 2008 Microchip Technology Inc
 */

#define OPT_BOARD_INTERNAL
#include "mpide/hardware/pic32/compiler/pic32-tools/pic32mx/include/p32xxxx.h"
#include "mpide/hardware/pic32/compiler/pic32-tools/pic32mx/include/proc/p32mx320f128l.h"
#include "mpide/hardware/pic32/compiler/pic32-tools/pic32mx/include/sys/attribs.h"
#include "mpide/hardware/pic32/cores/pic32/System_Defs.h"
#include "mpide/hardware/pic32/cores/pic32/p32_defs.h"
#include "mpide/hardware/pic32/cores/pic32/wiring.h"
#include "mpide/hardware/pic32/variants/Uno32/Board_Defs.h"

#include "serial.h"

static p32_uart* const    uart =                     (p32_uart *) _SER0_BASE;
static p32_regset* const  interrupt_flags =          (p32_regset *) &IFS0 + (_SER0_IRQ / 32);
static p32_regset* const  interrupt_enable_control = (p32_regset *) &IEC0 + (_SER0_IRQ / 32);
static const unsigned int error_bit =                1 << (_SER0_IRQ % 32);
static const unsigned int rx_bit =                   1 << ((_SER0_IRQ + 1) % 32);
static const unsigned int tx_bit =                   1 << ((_SER0_IRQ + 2) % 32);

static unsigned char tx_buffer[sizeof(struct motor_response)] __attribute__((aligned(4)));
static unsigned int tx_remaining;

static unsigned char rx_buffer[sizeof(struct motor_request)] __attribute__((aligned(4)));
static unsigned int rx_fill;

void
serial_open(unsigned int baud_rate)
{
  p32_regset *interrupt_priority_control;
  unsigned int irq_shift;

  interrupt_priority_control = ((p32_regset *) &IPC0) + (_SER0_VECTOR / 4);
  irq_shift = 8 * (_SER0_VECTOR % 4);

  /* UART initalization.  See Example 19-4.[1]  */
  interrupt_enable_control->clr = rx_bit | tx_bit | error_bit;
  interrupt_flags->clr = rx_bit | tx_bit | error_bit;
  interrupt_priority_control->clr = 0x1F << irq_shift;
  interrupt_priority_control->set = ((_SER0_IPL << 2) + _SER0_SPL) << irq_shift;
  interrupt_enable_control->set = rx_bit | tx_bit | error_bit;

  /* 8-bit data mode.  See Example 19-2.[1]  */
  uart->uxBrg.reg  = __PIC32_pbClk / 16 / baud_rate - 1; /* Example 19-1.[1]  */
  uart->uxMode.reg = (1 << _UARTMODE_ON);
  uart->uxSta.reg  = (1 << _UARTSTA_UTXEN) + (1 << _UARTSTA_URXEN);
}

void
serial_close(void)
{
  interrupt_enable_control->clr = error_bit | rx_bit | tx_bit;
  uart->uxMode.reg = 0;
}

void __ISR(_SER0_VECTOR, _SER0_IPL_ISR)
serial0_interrupt_handler(void)
{
  unsigned int flags = interrupt_flags->reg;

  if (!tx_remaining)
    {
      motor_generate_response((struct motor_response *) tx_buffer);
      tx_remaining = sizeof(tx_buffer);
    }

  /* Receive.  See Example 19-5.[1]  */
  if (0 != (flags & rx_bit))
    {
      rx_buffer[rx_fill++] = uart->uxRx.reg;

      /* Perform simple byte synchronization.  */
      if (rx_buffer[0] != 0xff)
        rx_fill = 0;

      if (rx_fill == sizeof(rx_buffer))
        {
          motor_process_request((const struct motor_request *) rx_buffer);
          rx_fill = 0;
        }

      interrupt_flags->clr = rx_bit;
    }

  /* Transmit.  See Section 19.3.[1]  */
  if (0 != (flags & tx_bit))
    {
      volatile unsigned char ch;
      ch = tx_buffer[sizeof(tx_buffer) - tx_remaining];

      interrupt_flags->clr = tx_bit;

      uart->uxTx.reg = ch;
      --tx_remaining;
    }

  if (0 != (flags & error_bit))
    interrupt_flags->clr = error_bit;
}
