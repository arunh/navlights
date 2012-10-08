set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)

cmake_minimum_required(VERSION 2.8)

project(navlights)

set(CMAKE_C_FLAGS "-std=c99 -Os")

set(ARDUINO_DEFAULT_BOARD nano328)
set(ARDUINO_DEFAULT_PORT /dev/tty.usbserial-A901C77G)

set(src_root src/main/c)
set(inc_root src/main/include)

set(navlights_SRCS
  ${src_root}/main.c  
  ${src_root}/uart.c
)

set(navlights_HDRS
  ${inc_root}/uart.h
)

include_directories(
  ${inc_root}
)

generate_arduino_firmware(navlights)

