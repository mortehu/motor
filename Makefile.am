noinst_PROGRAMS = motor.elf
noinst_LIBRARIES = libplatform.a

AM_CFLAGS = -ffunction-sections -fdata-sections -O3 -Wall
AM_CXXFLAGS = -ffunction-sections -fdata-sections -O3 -Wall -fno-exceptions -fno-rtti
AM_LDFLAGS = -Wl,--gc-sections

motor_elf_SOURCES = \
  main.cc \
  motor.cc

if SYSTEM_ARDUINO
AM_CPPFLAGS = -DSYSTEM_ARDUINO=1
endif

if SYSTEM_UNO32
noinst_DATA = motor.hex
AM_CPPFLAGS = -DSYSTEM_UNO32=1
motor_elf_LDADD = libplatform.a
libplatform_a_SOURCES = \
  mpide/hardware/pic32/cores/pic32/pins_arduino.c \
  mpide/hardware/pic32/cores/pic32/task_manager.c \
  mpide/hardware/pic32/cores/pic32/wiring.c \
  mpide/hardware/pic32/cores/pic32/wiring_analog.c \
  mpide/hardware/pic32/cores/pic32/wiring_digital.c \
  serial_uno32.c

SERIAL_PORT=/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_AE00DE7L-if00-port0
AVRDUDEFLAGS=-Cmpide/hardware/tools/avrdude.conf -c stk500v2 -p pic32 -P $(SERIAL_PORT) -b 115200 -v -U

install-data-local:
	$(AVRDUDE) $(AVRDUDEFLAGS) flash:w:motor.hex:i
endif

if SYSTEM_GNU
AM_CPPFLAGS = -DSYSTEM_GNU=1
motor_elf_LDADD = libplatform.a
libplatform_a_SOURCES = \
  hardware.cc \
  serial_linux.c
endif

motor.hex: motor.elf
	$(BIN2HEX) -a $<
