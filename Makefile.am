noinst_PROGRAMS = motor.elf
noinst_LIBRARIES = libplatform.a
noinst_DATA = motor.hex

AM_CFLAGS = -ffunction-sections -fdata-sections -O3 -Wall
AM_CXXFLAGS = -ffunction-sections -fdata-sections -O3 -Wall -fno-exceptions -fno-rtti
AM_LDFLAGS = --gc-sections

motor_elf_SOURCES = \
  main.cc \
  motor.cc \
  serial.c
motor_elf_LDADD = libplatform.a

libplatform_a_SOURCES = \
  mpide/hardware/pic32/cores/pic32/pins_arduino.c \
  mpide/hardware/pic32/cores/pic32/task_manager.c \
  mpide/hardware/pic32/cores/pic32/wiring.c \
  mpide/hardware/pic32/cores/pic32/wiring_analog.c \
  mpide/hardware/pic32/cores/pic32/wiring_digital.c

motor.hex: motor.elf
	$(BIN2HEX) -a $<
