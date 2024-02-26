## Hun Zeta

Library for using the zetaplus transceiver on the raspberry pi pico

### Adding the library 
`CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.28)

include(${CMAKE_SOURCE_DIR}/cmake/pico_sdk_import.cmake)
project(example C CXX ASM)

pico_sdk_init()

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(hun_zeta)

add_executable(example)

target_link_libraries(example pico_stdlib hun_zeta)
```
