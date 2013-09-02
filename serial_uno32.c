/* Specialized serial interface for motor controller.
 *
 * References:
 *
 *  1. PIC32MX3XX/4XX Datasheet, Preliminary, 2008 Microchip Technology Inc
 */

#include "serial.h"

#define OPT_BOARD_INTERNAL
#include <p32xxxx.h>
#include <proc/p32mx320f128l.h>
#include <sys/attribs.h>
#include <p32_defs.h>

#include "hardware.h"

static p32_uart* const    uart =                     (p32_uart *) _SER0_BASE;
static p32_regset* const  interrupt_flags =          (p32_regset *) &IFS0 + (_SER0_IRQ / 32);
static p32_regset* const  interrupt_enable_control = (p32_regset *) &IEC0 + (_SER0_IRQ / 32);
static const unsigned int error_bit =                1 << (_SER0_IRQ % 32);
static const unsigned int rx_bit =                   1 << ((_SER0_IRQ + 1) % 32);
static const unsigned int tx_bit =                   1 << ((_SER0_IRQ + 2) % 32);

static unsigned char tx_buffer[MAX_MESSAGE_SIZE] __attribute__((aligned(4)));
static uint16_t tx_size, tx_offset;

static rx_callback rx;
static tx_callback tx;

void
serial_open(enum serial_baud_rate baud_rate, rx_callback rx_, tx_callback tx_)
{
  p32_regset *interrupt_priority_control;
  unsigned int irq_shift, rate_num;
  unsigned int enable_flags = 0;

  rx = rx_;
  tx = tx_;

  interrupt_priority_control = ((p32_regset *) &IPC0) + (_SER0_VECTOR / 4);
  irq_shift = 8 * (_SER0_VECTOR % 4);

  /* UART initalization.  See Example 19-4.[1]  */
  interrupt_enable_control->clr = rx_bit | tx_bit | error_bit;
  interrupt_flags->clr = rx_bit | tx_bit | error_bit;
  interrupt_priority_control->clr = 0x1F << irq_shift;
  interrupt_priority_control->set = ((_SER0_IPL << 2) + _SER0_SPL) << irq_shift;
  interrupt_enable_control->set = rx_bit | tx_bit | error_bit;

  switch (baud_rate)
    {
    case SERIAL_57600: rate_num = 57600; break;
    default:
    case SERIAL_115200: rate_num = 115200;
    }

  /* 8-bit data mode.  See Example 19-2.[1]  */
  uart->uxBrg.reg  = __PIC32_pbClk / 16 / rate_num - 1; /* Example 19-1.[1]  */
  uart->uxMode.reg = (1 << _UARTMODE_ON);

  if (rx)
    enable_flags |= (1 << _UARTSTA_URXEN);
  if (tx)
    enable_flags |= (1 << _UARTSTA_UTXEN);

  uart->uxSta.reg = enable_flags;
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

  /* Receive.  See Example 19-5.[1]  */
  if (0 != (flags & rx_bit))
    {
      unsigned char ch;
      ch = uart->uxRx.reg;

      if (rx)
        rx (ch);

      interrupt_flags->clr = rx_bit;
    }

  /* Transmit.  See Section 19.3.[1]  */
  if (0 != (flags & tx_bit))
    {
      unsigned char ch;

      if (tx_offset == tx_size)
        {
          tx_offset = 0;
          tx_size = 0;
          tx(tx_buffer, &tx_size);
        }

      ch = tx_buffer[tx_offset++];

      interrupt_flags->clr = tx_bit;

      uart->uxTx.reg = ch;
    }

  if (0 != (flags & error_bit))
    interrupt_flags->clr = error_bit;
}
