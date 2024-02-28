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

### Example
```cpp
struct my_plain_struct {
    int32_t a;
    uint8_t b;
    uint8_t c;
    char d;
    int8_t e;
};

int main() {
    zeta::transceiver transceiver(uart0);
    transceiver.start(zeta::uart_baud_opt::UART_9600, 8, 1, 2);
    transceiver.set_mode(zeta::mode_t::READY);

    uint8_t my_buffer[20];
    transceiver.send_from(my_buffer, 20);

    transceiver.send<int>(2);

    std::vector<int> example = {3, 6, 1,5};
    transceiver.send_from(example);
    
    my_plain_struct inst;
    transceiver.send<my_plain_struct>(inst);

    auto result = transceiver.read<my_plain_struct>();

    transceiver.read_to(my_buffer, 4);
    return 0;
}


```