RC Navigation Lights Controller
===============================

Author
------
[Arun Horne](http://arunhorne.co.uk)

Overview
--------

This project provides a program that will run on an Arduino microcontrollers
to control a collection of lighting circuits, such as those used in an RC aeroplane.
The circuits can be controlled via the transmitter as the program receives inputs
from the onboard receiver.

Building
--------

The [CMake-Arduino](https://github.com/queezythegreat/arduino-cmake) build system
is used for this project. You will require a working version of CMake 2.8+ to 
be installed on your system. Edit CMakeLists.txt for your board and serial port,
then execute the following to configure:

    mkdir build
    cd build
    cmake ..

You can build the code and upload to your Arduino with

    make
    make navlights-upload 

Circuit Pins
------------

Input on:

    D2: Signal input from Rx

Output on:

    D4: Constant landing lights - white (leading edge)
    D5: Constant nav lights - constant red/green (wing tips), white (tail)
    D6: Strobe nav lights - flashing red (tail, belly) 

Switch States
-------------

Circuit is controlled by a single three position switch:

    Low  : D4 (off), D5 (off), D6 (off)
    Mid  : D4 (off), D5 (on ), D6 (on )  
    High : D4 (on ), D5 (on ), D6 (on )
	
Physical Circuit
----------------

To be documented, but LEDs should not be driven directly from microcontroller.
A transistor should be employed on each pin to switch current from a suitable
voltage source.	
	
Known Bugs
----------

* Strobe pattern can be interverted when turn-off then on. Depends on timing of
when switch was moved.
	
Future Work
-----------

### Eliminate Arduino libraries

Move code to use native AVR libc code. This really just involves implementing
native versions of digitalRead/digitalWrite. This is not necessary for
functionality, but would be an interesting exercise and may improve size of
the executable.

### Introduce a Second Strobe

    Dx: Strobe nav lights - flashing white (wingtips) 

D6 and Dx would flash out of sync for realism, but would both be activated by
the same switch state.

### Voltage Monitoring

Monitor voltage of supply battery and halt when drops below threshold. This is
useful for use with LiPo batteries which can be permanently damaged if voltage
drop too far.

    
