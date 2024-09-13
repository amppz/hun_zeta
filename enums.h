#pragma once

namespace zeta {
    enum class mode_t : uint8_t {
        RX = 1, // Only allows RX
        READY = 2, // Allows switching between RX or TX
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
}