noinst_PROGRAMS = motor.elf
noinst_LIBRARIES = libplatform.a

AM_CFLAGS = -ffunction-sections -fdata-sections -O3 -Wall
AM_CXXFLAGS = -ffunction-sections -fdata-sections -O3 -Wall -fno-exceptions -fno-rtti
AM_LDFLAGS = -Wl,--gc-sections

motor_elf_SOURCES = \
  main.cc \
  motor.cc
motor_elf_LDADD = libplatform.a

if SYSTEM_ARDUINO
noinst_DATA = motor.hex
AM_CPPFLAGS = -DSYSTEM_ARDUINO=1
libplatform_a_SOURCES = \
  $(WIRING_PATH)/wiring.c \
  $(WIRING_PATH)/wiring_analog.c \
  $(WIRING_PATH)/wiring_digital.c \
  serial_arduino.c

SERIAL_PORT=/dev/serial/by-id/usb-Arduino_*-if00
AVRDUDEFLAGS=$(AVRDUDE_ARGS) -P $(SERIAL_PORT) -b 115200 -v -U

install-data-local:
	$(AVRDUDE) $(AVRDUDEFLAGS) flash:w:motor.hex:i

motor.hex: motor.elf
	$(AM_V_GEN)$(OBJCOPY) -O ihex $< $@
endif

if SYSTEM_UNO32
noinst_DATA = motor.hex
AM_CPPFLAGS = -DSYSTEM_UNO32=1
libplatform_a_SOURCES = \
  $(WIRING_PATH)/pins_arduino.c \
  $(WIRING_PATH)/task_manager.c \
  $(WIRING_PATH)/wiring.c \
  $(WIRING_PATH)/wiring_analog.c \
  $(WIRING_PATH)/wiring_digital.c \
  serial_uno32.c

SERIAL_PORT=/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_AE00DE7L-if00-port0
AVRDUDEFLAGS=-C$(MPIDE_PATH)/hardware/tools/avrdude.conf $(AVRDUDE_ARGS) -P $(SERIAL_PORT) -b 115200 -v -U

install-data-local:
	$(AVRDUDE) $(AVRDUDEFLAGS) flash:w:motor.hex:i

motor.hex: motor.elf
	$(AM_V_GEN)$(BIN2HEX) $<
endif

if SYSTEM_GNU
AM_CPPFLAGS = -DSYSTEM_GNU=1
libplatform_a_SOURCES = \
  hardware.cc \
  serial_linux.c
endif
