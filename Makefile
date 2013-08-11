# System settings
CPUTYPE = 32MX320F128L
TOOLCHAIN_PREFIX = mpide/hardware/pic32/compiler/pic32-tools
AVRTOOLS_PREFIX = mpide/hardware/tools
SERIAL_PORT=/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_AE00DE7L-if00-port0
LDSCRIPT = mpide/hardware/pic32/cores/pic32/chipKIT-UNO32-application-$(CPUTYPE).ld

# Tool flags
CPPFLAGS = -DF_CPU=80000000L -Impide/hardware/pic32/cores/pic32 -Impide/hardware/pic32/variants/Uno32 -Impide/hardware/pic32/libraries/Wire/utility
CFLAGS = -O3 -mno-smart-io -Wall -mprocessor=$(CPUTYPE)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = -mprocessor=$(CPUTYPE)
LDFLAGS = \
  --gc-sections \
  -Lmpide/hardware/pic32/compiler/pic32-tools/pic32mx/lib/proc/$(CPUTYPE)
AVRDUDEFLAGS = -C$(AVRTOOLS_PREFIX)/avrdude.conf -c stk500v2 -p pic32 -P $(SERIAL_PORT) -b 115200 -v -U

# Tool aliases
CC = $(TOOLCHAIN_PREFIX)/bin/pic32-gcc
CXX = $(TOOLCHAIN_PREFIX)/bin/pic32-gcc
LD = $(TOOLCHAIN_PREFIX)/bin/pic32-ld
AR = $(TOOLCHAIN_PREFIX)/bin/pic32-ar
BIN2HEX = $(TOOLCHAIN_PREFIX)/bin/pic32-bin2hex

# Input files
AS_SOURCES = \
  mpide/hardware/pic32/cores/pic32/cpp-startup.S \
  mpide/hardware/pic32/cores/pic32/crti.S \
  mpide/hardware/pic32/cores/pic32/crtn.S
C_SOURCES = \
  mpide/hardware/pic32/cores/pic32/pins_arduino.c \
  mpide/hardware/pic32/cores/pic32/task_manager.c \
  mpide/hardware/pic32/cores/pic32/wiring.c \
  mpide/hardware/pic32/cores/pic32/wiring_analog.c \
  mpide/hardware/pic32/cores/pic32/wiring_digital.c \
  serial.c
CC_SOURCES = \
  main.cc \
  motor.cc
OBJECTS = $(AS_SOURCES:.S=.o) $(C_SOURCES:.c=.o) $(CC_SOURCES:.cc=.o)
LDADD = \
  mpide/hardware/pic32/compiler/pic32-tools/pic32mx/lib/libmchp_peripheral_$(CPUTYPE).a \
  mpide/hardware/pic32/compiler/pic32-tools/pic32mx/lib/libpic32.a

all: main.hex controller

install: main.hex
	avrdude $(AVRDUDEFLAGS) flash:w:main.hex:i

clean:
	rm -f $(OBJECTS)
	rm -f main.hex
	rm -f main.elf

main.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OUTPUT_OPTION) $(OBJECTS) $(LDADD) -T $(LDSCRIPT)

main.hex: main.elf
	$(BIN2HEX) -a main.elf

controller: controller.c
	gcc $(OUTPUT_OPTION) -Wall $<
