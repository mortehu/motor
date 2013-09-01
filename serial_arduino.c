#include "serial.h"

#include "protocol.h"
#include "Arduino.h"
#include "wiring_private.h"

#if !defined(UBRRH) && !defined(UBRRL)
#  if defined(UBRR0H) && defined(UBRR0L)
#    define UBRRH UBRR0H
#    define UBRRL UBRR0L
#    define UCSRA UCSR0A
#    define UCSRB UCSR0B
#    define UDR   UDR0
#    define RXEN  RXEN0
#    define TXEN  TXEN0
#    define RXCIE RXCIE0
#    define UDRIE UDRIE0
#    define U2X   U2X0
#  else
#    error "Serial support macros missing"
#  endif
#endif

#if defined(USART_RX_vect)
#  define RX_SIGNAL USART_RX_vect
#elif defined(SIG_USART0_RECV)
#  define RX_SIGNAL SIG_USART0_RECV
#elif defined(SIG_UART0_RECV)
#  define RX_SIGNAL SIG_UART0_RECV
#elif defined(USART0_RX_vect)
#  define RX_SIGNAL USART0_RX_vect
#elif defined(SIG_UART_RECV)
#  define RX_SIGNAL SIG_UART_RECV
#else
#  error "Missing serial RX signal macro"
#endif

#if defined(UART0_UDRE_vect)
#  define TX_SIGNAL UART0_UDRE_vect
#elif defined(UART_UDRE_vect)
#  define TX_SIGNAL UART_UDRE_vect
#elif defined(USART0_UDRE_vect)
#  define TX_SIGNAL USART0_UDRE_vect
#elif defined(USART_UDRE_vect)
#  define TX_SIGNAL USART_UDRE_vect
#else
#  error "Missing serial TX ISR macro"
#endif

static unsigned char tx_buffer[sizeof(struct motor_response)] __attribute__((aligned(4)));
static unsigned int tx_remaining;

static unsigned char rx_buffer[sizeof(struct motor_request)] __attribute__((aligned(4)));
static unsigned int rx_fill;

void
serial_open(uint32_t baud_rate)
{
  uint16_t baud_setting;

  /* Compatibility exception for:
   *  - bootloader on Arduino Duemilanove and older
   *  - firmware on 8U2 on Arduino Uno and Mega 2560
   */
  if (F_CPU == 16000000UL && baud_rate == 57600)
    goto no_u2x;

  baud_setting = (F_CPU / 4 / baud_rate - 1) / 2;

  if (baud_setting >= 0x1000)
    {
no_u2x:
      UCSRA = 0;
      baud_setting = (F_CPU / 8 / baud_rate - 1) / 2;
    }
  else
    UCSRA = 1 << U2X;

  /* Assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register) */
  UBRRH = baud_setting >> 8;
  UBRRL = baud_setting;

  sbi(UCSRB, RXEN);
  sbi(UCSRB, TXEN);
  sbi(UCSRB, RXCIE);
  sbi(UCSRB, UDRIE);
}

void
serial_close(void)
{
  cbi(UCSRB, RXEN);
  cbi(UCSRB, TXEN);
  cbi(UCSRB, RXCIE);
  cbi(UCSRB, UDRIE);
}

SIGNAL(RX_SIGNAL)
{
  unsigned char ch = UDR;

  rx_buffer[rx_fill++] = ch;

  if (rx_fill == sizeof(rx_buffer))
    {
      motor_process_request((const struct motor_request *) rx_buffer);
      rx_fill = 0;
    }
  else if (rx_buffer[0] != 0xff)
    rx_fill = 0;
}

SIGNAL(TX_SIGNAL)
{
  unsigned char ch;

  if (!tx_remaining)
    {
      motor_generate_response((struct motor_response *) tx_buffer);
      tx_remaining = sizeof(tx_buffer);
    }

  ch = tx_buffer[sizeof(tx_buffer) - tx_remaining];
  --tx_remaining;

  UDR = ch;
}
