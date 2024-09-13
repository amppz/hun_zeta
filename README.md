## Hun Zeta

Library for using the zetaplus transceiver on the raspberry pi pico

### Adding the library 
`CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.28)

include(/path/to/pico_sdk_import.cmake)
project(example C CXX ASM)

pico_sdk_init()

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(hun_zeta)

add_executable(example)
target_link_libraries(example pico_stdlib hun_zeta)

pico_enable_stdio_usb(example 1)
pico_enable_stdio_uart(example 0)

pico_add_extra_outputs(example)
```

### Example code
```cpp
#include <pico/stdlib.h>
#include <cstdio>
#include "hun_zeta/transceiver.h"

#define PIN_SDN 2
#define PIN_RX 1
#define PIN_TX 0

void print_arr(uint8_t const *data, uint count, const char *format, const char *prefix, const char *suffix);

void output_response(zeta::response_t const &resp) {
    switch (resp.packet_type) {
        case zeta::packet_type_t::RSSI: {
            std::printf("RSSI: %u\n", resp.rssi);
            return;
        }
        case zeta::packet_type_t::FIRMWARE : {
            std::printf("Firmware: %s\n", resp.firmware_str);
            return;
        }
        case zeta::packet_type_t::CONFIG: {
            auto &cfg = resp.config;
            std::printf("Channel: %x, Mode: %x, RF Baud: %lu, RF Power: %u, Sync Bytes: ",
                   cfg.channel_no, (uint8_t) cfg.mode, zeta::rf_baud_opt_to_value(cfg.rf_baud_opt), cfg.rf_power);
            print_arr(cfg.sync_bytes, 4, "%x", "[", "]\n");
            return;
        }
        case zeta::packet_type_t::DATA: {
            auto &read = resp.read;
            std::printf("RSSI: %d, Length: %d, Data: ", read.rssi, read.length);
            print_arr(resp.read.data, read.length, "%x", "[", "]\n");
            return;
        }
        default:
            printf("Received unhandled type: %x\n", (uint8_t) resp.packet_type);
            return;
    }
}

int main() {
    constexpr zeta::config_t cfg{
            .m_baud_rate = zeta::uart_baud_opt::UART_19200,
            .pin_rx = PIN_RX,
            .pin_tx = PIN_TX,
            .pin_shutdown = PIN_SDN,
            .receive_bytes = 2,
            .receive_channel = 0};

    stdio_init_all();
    sleep_ms(5000);

    std::puts("Started.");

    zeta::transceiver xceiver(uart0, cfg);

    std::puts("Requesting rssi...");
    xceiver.request_rssi();
    output_response(xceiver.read());

    std::puts("Requesting device fw...");
    xceiver.request_firmware();
    output_response(xceiver.read());

    std::puts("Requesting device config...");
    xceiver.request_device_config();
    output_response(xceiver.read());
    
    xceiver.set_output_channel(1);
    xceiver.send<uint64_t>(time_us_64());
}

void print_arr(uint8_t const *data, uint count, const char *format, const char *prefix, const char *suffix) {
    if (count == 0) {
        printf("%s", prefix);
        printf("%s", suffix);
        return;
    }
    printf("%s", prefix);
    for (auto i = 0; i < (count - 1); ++i) {
        printf(format, data[i]);
        printf(", ");
    }
    printf(format, data[count - 1]);
    printf("%s", suffix);
}

```