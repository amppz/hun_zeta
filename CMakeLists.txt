cmake_minimum_required(VERSION 3.21)

include(${CMAKE_SOURCE_DIR}/cmake/pico_sdk_import.cmake)
project(hun_zeta C CXX ASM)

pico_sdk_init()

set(CMAKE_CXX_STANDARD 20)

add_library(hun_zeta transceiver.cpp transceiver.h)

target_link_libraries(hun_zeta PUBLIC pico_stdlib)

