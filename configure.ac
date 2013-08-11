AC_INIT(motor,1)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CANONICAL_HOST

SYSTEM_UNO32=0
SYSTEM_GNU=0

case "$host" in
  *-arduino-*)
    AC_MSG_ERROR([Arduino support is missing.])
    ;;

  *-pic32-*)
    TOOLCHAIN_PREFIX=mpide/hardware/pic32/compiler/pic32-tools
    CPPFLAGS="$CPPFLAGS -Impide/hardware/pic32/libraries/Wire/utility"
    CPPFLAGS="$CPPFLAGS -Impide/hardware/pic32/cores/pic32"

    case "$host_cpu" in
      mips)
        CPUTYPE=32MX320F128L
        CFLAGS="$CFLAGS -mno-smart-io -mprocessor=$CPUTYPE"
        CXXFLAGS="$CXXFLAGS -mno-smart-io -mprocessor=$CPUTYPE"
        CCASFLAGS="$CCASFLAGS -mprocessor=$(CPUTYPE)"
        CPPFLAGS="$CPPFLAGS -Impide/hardware/pic32/variants/Uno32"
        CPPFLAGS="$CPPFLAGS -Lmpide/hardware/pic32/compiler/pic32-tools/pic32mx/lib/proc/$CPUTYPE"
        LDFLAGS="$LDFLAGS -Tmpide/hardware/pic32/cores/pic32/chipKIT-UNO32-application-$CPUTYPE.ld"

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

AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AS

AC_C_BIGENDIAN

AC_PATH_PROG(AVRDUDE, avrdude)

AC_SUBST(BIN2HEX)
AC_SUBST(CPUTYPE)

AM_CONDITIONAL(SYSTEM_UNO32, [test $SYSTEM_UNO32 = 1])
AM_CONDITIONAL(SYSTEM_GNU, [test $SYSTEM_GNU = 1])

AC_OUTPUT(Makefile)
