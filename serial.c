#define OPT_BOARD_INTERNAL
#include "mpide/hardware/pic32/compiler/pic32-tools/pic32mx/include/p32xxxx.h"
#include "mpide/hardware/pic32/compiler/pic32-tools/pic32mx/include/proc/p32mx320f128l.h"
#include "mpide/hardware/pic32/compiler/pic32-tools/pic32mx/include/sys/attribs.h"
#include "mpide/hardware/pic32/cores/pic32/System_Defs.h"
#include "mpide/hardware/pic32/cores/pic32/p32_defs.h"
#include "mpide/hardware/pic32/cores/pic32/wiring.h"
#include "mpide/hardware/pic32/variants/Uno32/Board_Defs.h"

static p32_uart*    uart;
static p32_regset*  interrupt_flags;
static p32_regset*  interrupt_enable_control;
static unsigned int error_bit, rx_bit, tx_bit;

void serial_init(void)
{
  uart = (p32_uart *) _SER0_BASE;

  interrupt_flags = ((p32_regset *)&IFS0) + (_SER0_IRQ / 32);
  interrupt_enable_control = ((p32_regset *)&IEC0) + (_SER0_IRQ / 32);

  error_bit = 1 << (_SER0_IRQ % 32);
  rx_bit =    1 << ((_SER0_IRQ + 1) % 32);
  tx_bit =    1 << ((_SER0_IRQ + 2) % 32);
}

void serial_open(unsigned int baud_rate)
{
  p32_regset *interrupt_priority_control;
  unsigned int irq_shift;

  interrupt_priority_control = ((p32_regset *) &IPC0) + (_SER0_VECTOR / 4);

  irq_shift = 8 * (_SER0_VECTOR % 4);

  interrupt_priority_control->clr = 0x1F << irq_shift;
  interrupt_priority_control->set = ((_SER0_IPL << 2) + _SER0_SPL) << irq_shift;

  interrupt_flags->clr = rx_bit | tx_bit | error_bit;
  interrupt_enable_control->clr = rx_bit | tx_bit | error_bit;
  interrupt_enable_control->set = rx_bit;

  uart->uxBrg.reg  = __PIC32_pbClk / 16 / baud_rate - 1;
  uart->uxSta.reg = 0;
  uart->uxMode.reg = (1 << _UARTMODE_ON);
  uart->uxSta.reg  = (1 << _UARTSTA_UTXEN) + (1 << _UARTSTA_URXEN);
}

void serial_close(void)
{
  interrupt_enable_control->clr = error_bit | rx_bit | tx_bit;
  uart->uxMode.reg = 0;
}

void serial_write(char ch)
{
  while ((uart->uxSta.reg & (1 << _UARTSTA_TMRT)) == 0)   //check the TRMT bit
    ;

  uart->uxTx.reg = ch;
}

void __ISR(_SER0_VECTOR, _SER0_IPL_ISR) IntSer0Handler(void)
{
  /* Receive */
  if ((interrupt_flags->reg & rx_bit) != 0)
    {
      char ch = uart->uxRx.reg;
#if 0
      unsigned int bufIndex = (rx_buffer.head + 1) % RX_BUFFER_SIZE;

      if (bufIndex != rx_buffer.tail)
        {
          rx_buffer.buffer[rx_buffer.head] = ch;
          rx_buffer.head = bufIndex;
        }
#endif

      /* Clear the interrupt flag.  */
      interrupt_flags->clr = rx_bit;
    }

  /* Transmit */
  if ((interrupt_flags->reg & tx_bit) != 0)
    {
      /* Clear the interrupt flag.  */
      interrupt_flags->clr = tx_bit;
    }
}
