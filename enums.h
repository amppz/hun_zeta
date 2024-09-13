#pragma once

#include <cstdint>

namespace zeta {
    enum class mode_t : uint8_t {
        RX = 1,
        // Default
        READY = 2,
        SLEEP = 3
    };
    enum class rf_baud_opt : uint8_t {
        RF_4800 = 1,
        RF_9600 = 2,
        RF_38400 = 3,
        RF_128000 = 4,
        RF_256000 = 5,
        RF_500000 = 6,
    };

    enum class uart_baud_opt : uint8_t {
        UART_9600 = 0,
        // Default
        UART_19200 = 1,
        UART_28800 = 2,
        UART_38400 = 3,
        UART_57600 = 4,
    };

    enum class packet_type_t : uint8_t {
        RSSI = 'Q',
        READ = 'R',
        CONFIG = '?',
        FIRMWARE = 'Z',
        TRANSMIT = 'T',
        UNKNOWN = 255
    };

    static constexpr uint16_t uart_baud_opt_to_value(zeta::uart_baud_opt baud) noexcept {
        switch (baud) {
            case zeta::uart_baud_opt::UART_9600:
                return 9600;
            case zeta::uart_baud_opt::UART_19200:
                return 19200;
            case zeta::uart_baud_opt::UART_28800:
                return 28880;
            case zeta::uart_baud_opt::UART_38400:
                return 38400;
            case zeta::uart_baud_opt::UART_57600:
                return 57600;
            default:
                return 0;
        }
    }

    static constexpr uint32_t rf_baud_opt_to_value(zeta::rf_baud_opt baud) noexcept {
        switch (baud) {
            case rf_baud_opt::RF_4800:
                return 4800;
            case rf_baud_opt::RF_9600:
                return 9600;
            case rf_baud_opt::RF_38400:
                return 38400;
            case rf_baud_opt::RF_128000:
                return 128000;
            case rf_baud_opt::RF_256000:
                return 256000;
            case rf_baud_opt::RF_500000:
                return 500000;
            default:
                return 0;
        }
    }


}