#include "serial.h"

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

static unsigned char tx_buffer[MAX_MESSAGE_SIZE] __attribute__((aligned(4)));
static uint16_t tx_size, tx_offset;

static rx_callback rx;
static tx_callback tx;

void
serial_open(enum serial_baud_rate baud_rate, rx_callback rx_, tx_callback tx_)
{
  /* Allow missing the requested baud rate by 2%.  */
#define BAUD_TOL 2

  switch (baud_rate)
    {
    case SERIAL_57600:

#define BAUD 57600
#include <util/setbaud.h>
      UBRRH = UBRRH_VALUE;
      UBRRL = UBRRL_VALUE;
#if USE_2X
      UCSRA |= (1 << U2X);
#else
      UCSRA &= ~(1 << U2X);
#endif
#undef BAUD

      break;

    default:
    case SERIAL_115200:

#define BAUD 115200
#include <util/setbaud.h>
      UBRRH = UBRRH_VALUE;
      UBRRL = UBRRL_VALUE;
#if USE_2X
      UCSRA |= (1 << U2X);
#else
      UCSRA &= ~(1 << U2X);
#endif
#undef BAUD

      break;
    }

#undef BAUD_TOL

  rx = rx_;
  tx = tx_;

  sbi(UCSRB, RXEN);
  sbi(UCSRB, TXEN);
  if (rx_)
    sbi(UCSRB, RXCIE);
  else
    cbi(UCSRB, RXCIE);
  if (tx_)
    sbi(UCSRB, UDRIE);
  else
    cbi(UCSRB, UDRIE);
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
  rx(UDR);
}

SIGNAL(TX_SIGNAL)
{
  unsigned char ch;

  if (tx_offset == tx_size)
    {
      tx_offset = 0;
      tx_size = 0;
      tx(tx_buffer, &tx_size);
    }

  ch = tx_buffer[tx_offset++];

  UDR = ch;
}
