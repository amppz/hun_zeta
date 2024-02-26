#pragma once
#include <pico/stdlib.h>
#include <span>
#include <cstring>
#include "enums.h"

namespace zeta {
    class transceiver {
    public:
        explicit transceiver(uart_inst* uart_inst);
        ~transceiver();

        void start(uart_baud_opt baud_rate_opt, uint pin_shutdown);

        template<typename Type>
        void send_from(std::span<Type> data_span) {
            auto const size = data_span.size_bytes();
            uint8_t data[size + 5];
            data[0] = 'A';
            data[1] = 'T';
            data[2] = 'S';
            data[3] = m_channel;
            data[4] = size;
            uart_write_blocking(m_uart, data, size + 5);
        }

        template<typename T> requires std::is_pod_v<std::remove_cvref_t<T>>
        void send(T&& plain_data) {
            auto constexpr size = sizeof(std::remove_cvref_t<T>);
            uint8_t data[size + 5];
            data[0] = 'A';
            data[1] = 'T';
            data[2] = 'S';
            data[3] = m_channel;
            data[4] = size;
            std::memcpy(data + 5, &plain_data, size);
            uart_write_blocking(m_uart, data, size + 5);
        }

        void set_mode(zeta::mode_t mode);

        /**
         * \brief
         * \param rate use the enum
         * \return true on success
         */
        void set_uart_baud_rate(zeta::uart_baud_opt rate);

        /**
         * \brief sets the rf baud rate and restarts the transceiver
         * \return
         */
        void set_rf_baud_rate(zeta::rf_baud_opt rate);


        void set_rf_output_power(uint8_t power_level);

        /**
         * \brief
         * \param channel must be less than 16
         */
        void set_transmission_channel(uint8_t channel);

        void read_to(uint8_t* data, size_t size);

        template <typename T> requires std::is_pod_v<T>
        T read() {
            uint8_t d[sizeof(T)];
            uart_read_blocking(m_uart, d, sizeof(T));
            return *reinterpret_cast<uint8_t*>(d);
        }


    private:
        void restart();


    private:
        uart_inst* m_uart;
        uint8_t m_channel{};
        uint m_pin_shutdown{};

    };


}
