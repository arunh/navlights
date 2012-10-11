################################################################################
# NAVLIGHTS BUILD
################################################################################

cmake_minimum_required(VERSION 2.8)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
project(ArduinoNavLights)

################################################################################
# Compiler options
################################################################################

set(CMAKE_C_FLAGS "-std=c99 -Os")


################################################################################
# Board options
################################################################################

set(ARDUINO_DEFAULT_BOARD nano328)
set(ARDUINO_DEFAULT_PORT /dev/tty.usbserial-A901C77G)

#Set some defines to help Eclipse correctly resolve imports/definitions.
#Have a look in avr/io.h to deduce correct define for your board.
#WARNING: This is very chip specific
if (CMAKE_EXTRA_GENERATOR)
  set(ARDUINO_IO_DEFINE __AVR_ATmega328P__)
  set(ARDUINO_F_CPU 16000000L)
  message("-- Added preprocessor define for generator: ${ARDUINO_IO_DEFINE}")
  add_definitions( -D${ARDUINO_IO_DEFINE} -DF_CPU=${ARDUINO_F_CPU})
endif()


################################################################################
# Navlights firmware
################################################################################

set(src_root src/main/c)
set(inc_root src/main/include)

set(navlights_SRCS
  ${src_root}/main.c  
  ${src_root}/uart.c
  ${src_root}/time.c
  ${src_root}/digital.c
)

set(navlights_HDRS
  ${inc_root}/uart.h
  ${inc_root}/time.h
  ${inc_root}/digital.h
)

include_directories(
  ${inc_root}
)

generate_arduino_firmware(navlights NATIVE_AVR)
