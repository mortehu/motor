Brushless DC electric motor controller
======================================

Implements a controller for three phase BLDC motors.

If you're building this package from a Git working copy rather than a
distribution, you must run

    autoreconf -f -i

to generate configure, Makefile.in and other files needed to build.

To build this package for chipKIT Uno32, run:

    wget https://github.com/downloads/chipKIT32/chipKIT32-MAX/mpide-0023-linux-20120903.tgz
    tar zxf mpide-0023-linux-20120903.tgz
    ./configure --host=mips-pic32-none \
      --with-mpide-path=mpide-0023-linux-20120903
    make

To build this package for Arduino Mega 2560, run:

    ./configure --host=avr-arduino_mega2560-none
    make

To build this package for Arduino Uno, run:

    ./configure --host=avr-arduino_uno-none
    make
