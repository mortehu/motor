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
serial_open(uint32_t baud_rate, rx_callback rx_, tx_callback tx_)
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
