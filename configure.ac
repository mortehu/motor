AC_INIT(motor,1)
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CANONICAL_HOST

SYSTEM_ARDUINO=0
SYSTEM_UNO32=0
SYSTEM_GNU=0

WIRING_DEFAULT_PATH=wiring
AVRDUDE_ARGS=

AC_ARG_WITH([mpide-path],
            [AS_HELP_STRING([--with-mpide-path],
                            [location of the MPIDE])],
            [MPIDE_PATH="$withval"],
            [MPIDE_PATH="mpide"])

case "$host" in
  avr-*-*)
    CC=avr-gcc
    CXX=avr-c++
    LD=avr-ld
    RANLIB=avr-ranlib

    case "$host_vendor" in
      hercules_3_1_ls)
        CPPFLAGS="$CPPFLAGS -I/usr/share/arduino/hardware/arduino/cores/arduino"
        CPPFLAGS="$CPPFLAGS -I/usr/share/arduino/hardware/arduino/variants/mega"
        CPPFLAGS="$CPPFLAGS -DHERCULES3_1"
        CFLAGS="$CFLAGS -mmcu=atmega2560"
        CXXFLAGS="$CXXFLAGS -mmcu=atmega2560"
        WIRING_DEFAULT_PATH=/usr/share/arduino/hardware/arduino/cores/arduino
        # TODO(mortehu): Program using this command:
        # avrdude -c avrispmkII -p m2560 -P usb -v -U flash:w:$(FILE):i
        AVRDUDE_ARGS="-c stk500v2 -p m2560"

        AC_DEFINE(F_CPU, 8000000UL)
        SYSTEM_ARDUINO=1
        ;;

      arduino_mega2560)
        CPPFLAGS="$CPPFLAGS -I/usr/share/arduino/hardware/arduino/cores/arduino"
        CPPFLAGS="$CPPFLAGS -I/usr/share/arduino/hardware/arduino/variants/mega"
        CFLAGS="$CFLAGS -mmcu=atmega2560"
        CXXFLAGS="$CXXFLAGS -mmcu=atmega2560"
        WIRING_DEFAULT_PATH=/usr/share/arduino/hardware/arduino/cores/arduino
        AVRDUDE_ARGS="-c stk500v2 -p m2560"

        AC_DEFINE(F_CPU, 16000000UL)
        SYSTEM_ARDUINO=1
        ;;

      arduino_uno)
        CPPFLAGS="$CPPFLAGS -I/usr/share/arduino/hardware/arduino/cores/arduino"
        CPPFLAGS="$CPPFLAGS -I/usr/share/arduino/hardware/arduino/variants/standard"
        CFLAGS="$CFLAGS -mmcu=atmega328p"
        CXXFLAGS="$CXXFLAGS -mmcu=atmega328p"
        WIRING_DEFAULT_PATH=/usr/share/arduino/hardware/arduino/cores/arduino
        AVRDUDE_ARGS="-c arduino -p m328p"

        AC_DEFINE(F_CPU, 16000000UL)
        SYSTEM_ARDUINO=1
        ;;

      *)
        AC_MSG_ERROR([Unsupported AVR host vendor '$host_vendor'])
        ;;
    esac
    ;;

  *-pic32-*)
    WIRING_DEFAULT_PATH="$MPIDE_PATH/hardware/pic32/cores/pic32"
    TOOLCHAIN_PREFIX="$MPIDE_PATH/hardware/pic32/compiler/pic32-tools"
    CPPFLAGS="$CPPFLAGS -I$MPIDE_PATH/hardware/pic32/libraries/Wire/utility"
    CPPFLAGS="$CPPFLAGS -I$MPIDE_PATH/hardware/pic32/cores/pic32"
    CPPFLAGS="$CPPFLAGS -I$MPIDE_PATH/hardware/pic32/variants/Uno32"
    CPPFLAGS="$CPPFLAGS -I$MPIDE_PATH/pic32/compiler/pic32-tools/pic32mx/include"

    case "$host_cpu" in
      mips)
        CPUTYPE=32MX320F128L
        CFLAGS="$CFLAGS -mno-smart-io -mprocessor=$CPUTYPE"
        CXXFLAGS="$CXXFLAGS -mno-smart-io -mprocessor=$CPUTYPE"
        CCASFLAGS="$CCASFLAGS -mprocessor=$(CPUTYPE)"
        CPPFLAGS="$CPPFLAGS -I$MPIDE_PATH/hardware/pic32/variants/Uno32"
        LDFLAGS="$CPPFLAGS -L$MPIDE_PATH/hardware/pic32/compiler/pic32-tools/pic32mx/lib/proc/$CPUTYPE"
        LDFLAGS="$LDFLAGS -T$MPIDE_PATH/hardware/pic32/cores/pic32/chipKIT-UNO32-application-$CPUTYPE.ld"
        AVRDUDE_ARGS="-c stk500v2 -p pic32"

        BIN2HEX=$TOOLCHAIN_PREFIX/bin/pic32-bin2hex
        CC=$TOOLCHAIN_PREFIX/bin/pic32-gcc
        CXX=$TOOLCHAIN_PREFIX/bin/pic32-c++
        LD=$TOOLCHAIN_PREFIX/bin/pic32-ld
        RANLIB=$TOOLCHAIN_PREFIX/bin/pic32-ranlib

        AC_DEFINE(F_CPU, 80000000L)
        SYSTEM_UNO32=1
        ;;

      *)
        AC_MSG_ERROR([Unsupported PIC32 CPU type '$host_cpu'])
        ;;
    esac
    ;;

  *-gnu)
    SYSTEM_GNU=1
    ;;

  *)
    AC_MSG_ERROR([Unsupported host '$host'])
    ;;
esac

AC_ARG_WITH([wiring-path],
            [AS_HELP_STRING([--with-wiring-path],
                            [location of the Wiring source files and headers])],
            [WIRING_PATH="$withval"],
            [WIRING_PATH="$WIRING_DEFAULT_PATH"])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AS
AM_PROG_AR

AC_C_BIGENDIAN

AC_PATH_PROG(AVRDUDE, avrdude)
AC_PATH_PROG(OBJCOPY, objcopy)

AC_SUBST(BIN2HEX)
AC_SUBST(CPUTYPE)
AC_SUBST(AVRDUDE_ARGS)

AC_SUBST(MPIDE_PATH)
AC_SUBST(WIRING_PATH)
AM_CONDITIONAL(SYSTEM_ARDUINO, [test $SYSTEM_ARDUINO = 1])
AM_CONDITIONAL(SYSTEM_UNO32, [test $SYSTEM_UNO32 = 1])
AM_CONDITIONAL(SYSTEM_GNU, [test $SYSTEM_GNU = 1])

AC_OUTPUT(Makefile)
