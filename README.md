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
#include <iostream>
#include <pico/stdlib.h>
#include <vector>
#include "hun_zeta/transceiver.h"

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
    transceiver.set_transmission_channel(13);

    // Send from a byte buffer
    uint8_t my_buffer[20];
    transceiver.send_from(my_buffer, 20);

    // Send a specific value
    transceiver.send<int>(2);

    // Send from a standard-library container
    std::vector<uint16_t> example = {3, 6, 1, 5};
    transceiver.send_from(example);

    // Send from a custom structure
    my_plain_struct inst{112304, 2, 4, 'A', 42};
    transceiver.send<my_plain_struct>(inst);

    // Read out a response
    auto resp = transceiver.read<my_plain_struct>();

    switch (resp.packet_type) {
        case zeta::packet_type_t::TRANSMIT:
        case zeta::packet_type_t::RSSI: {
            printf("RSSI: %u", resp.rssi);
            break;
        }
        case zeta::packet_type_t::READ: {
            auto my_struct = resp.read.value;
            printf("Read: %ld", my_struct.a);
            break;
        }
        case zeta::packet_type_t::CONFIG: {
            auto &cfg = resp.config;
            printf("Config: (CH: %u, PW: %u, RFB%lu, %u)",
                   cfg.channel_no,
                   cfg.rf_power,
                   zeta::rf_baud_opt_to_value(cfg.rf_baud_opt),
                   (uint8_t) cfg.mode);
            break;
        }
        case zeta::packet_type_t::FIRMWARE: {
            printf("Firmware: %s", resp.firmware_str);
            break;
        }
        default: {
            perror("Unknown packet type received");
            break;
        }
    }

    return 0;
}

```