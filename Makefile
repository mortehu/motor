CPUTYPE=32MX795F512L
TOOLCHAIN_PREFIX=mpide/hardware/pic32/compiler/pic32-tools
AVRTOOLS_PREFIX=mpide/hardware/tools
SERIAL_PORT=/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_AE00DE7L-if00-port0
LDSCRIPT=mpide/hardware/pic32/cores/pic32/chipKIT-MAX32-application-$(CPUTYPE).ld

CPPFLAGS=-DF_CPU=80000000L
CFLAGS=-mno-smart-io -fno-exceptions -ffunction-sections -fdata-sections -mdebugger -Wcast-align -fno-short-double -mprocessor=$(CPUTYPE)
ASFLAGS=-mprocessor=32MX795F512L
LDFLAGS=-Wl,--gc-sections -mdebugger -mprocessor=$(CPUTYPE)

CC=$(TOOLCHAIN_PREFIX)/bin/pic32-gcc
LD=$(CC)
AR=$(TOOLCHAIN_PREFIX)/bin/pic32-ar
BIN2HEX=$(TOOLCHAIN_PREFIX)/bin/pic32-bin2hex

AVRDUDEFLAGS=-C$(AVRTOOLS_PREFIX)/avrdude.conf -c stk500v2 -p pic32 -P $(SERIAL_PORT) -b 115200 -v -U

C_SOURCES = \
  mpide/hardware/pic32/variants/Max32/Board_Data.c \
  main.c
OBJECTS = $(C_SOURCES:.c=.o)

all: main.hex

install: main.hex
	avrdude $(AVRDUDEFLAGS) flash:w:main.hex:i

clean:
	rm -f *.o
	rm -f main.hex
	rm -f main.elf

crt0.o: mpide/hardware/pic32/cores/pic32/cpp-startup.S
	$(COMPILE.S) $< $(OUTPUT_OPTION)

crti.o: mpide/hardware/pic32/cores/pic32/crti.S
	$(COMPILE.S) $< $(OUTPUT_OPTION)

crtn.o: mpide/hardware/pic32/cores/pic32/crtn.S
	$(COMPILE.S) $< $(OUTPUT_OPTION)

startup.a: crt0.o crti.o crtn.o
	$(AR) crs $@ crt0.o crti.o crtn.o

main.elf: main.o startup.a
	$(LD) $(LDFLAGS) $(OUTPUT_OPTION) main.o startup.a -lm -T $(LDSCRIPT)

main.hex: main.elf
	$(BIN2HEX) -a main.elf
