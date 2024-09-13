#pragma once

#include <pico/stdlib.h>
#include <ranges>
#include <span>
#include <cstring>

#include "enums.h"

namespace zeta {

#pragma pack(1)
    struct device_config_t {
        zeta::mode_t mode;
        zeta::rf_baud_opt rf_baud_opt;
        uint8_t rf_power;
        uint8_t sync_bytes[4];
        uint8_t channel_no;
    };

    struct response_t {
        packet_type_t packet_type{packet_type_t::UNKNOWN};
        union {
            uint8_t rssi;
            device_config_t config;
            int8_t firmware_str[12];
            struct {
                uint8_t length{};
                uint8_t rssi{};
                uint8_t data[64];
            } read;

        };
    };
#pragma pack()
    struct config_t {
        uart_baud_opt m_baud_rate;
        uint pin_rx;
        uint pin_tx;
        uint pin_shutdown;
        uint8_t receive_bytes;
        uint8_t receive_channel;
    };

    /**
     * Class for using a ZETAPLUS transceiver
     * ALL functions EXCEPT `set_transmission_channel` block on UART
     * more information here: https://www.rfsolutions.co.uk/downloads/652412b497cb1739DS-ZETAPLUS-8.pdf
     * archived version: https://web.archive.org/web/20240302200730/https://www.rfsolutions.co.uk/downloads/652412b497cb1739DS-ZETAPLUS-8.pdf
     */
    class transceiver {
    public:
        /**
         * \brief constructor for if the UART is to be wholly controlled by this class
         * @param uart_inst - uninitialised uart instance
         * @param config
         */
        explicit transceiver(uart_inst *uart_inst, config_t const& config) noexcept;
        /**
         * \brief constructor for if UART and pins are initialised outside of this class
         * @param uart - initialised uart instance
         * @param baud_rate_option - baud rate option
         * @param pin_shutdown - ZETAPLUS shutdown pin
         * @param receive_bytes - ZETAPLUS bytes to receive
         * @param channel - ZETAPLUS rf receive channel
         */
        explicit transceiver(uart_inst* uart, uart_baud_opt baud_rate_option, uint pin_shutdown, uint8_t receive_bytes, uint8_t channel) noexcept;

        ~transceiver() noexcept;

        /**
         * Configure RX mode
         * @param byte_count - max number of bytes reads should receive, max is 64
         * @param receive_channel - channel that read from
         * All messages received will be truncated to, or padded to `byte_count` bytes.
         * @tag completes on ZETAPLUS
         */
        void configure_rx(uint8_t byte_count, uint8_t receive_channel) noexcept;

        /**
         * @brief sends data to ZETAPLUS
         * @param src - source of data
         * @param byte_count - number of bytes to send from it
         * @tag Completes on ZETAPLUS
         */
        void send_from(const void *src, size_t byte_count) noexcept;

        /**
         * @brief sends data to ZETAPLUS
         * @param range - a contiguous range, for example std::vector
         * @tag Completes on ZETAPLUS
         */
        void send_from(std::ranges::contiguous_range auto const& range) noexcept {
            auto const size = std::ranges::size(range);
            uint8_t data[size + 5];
            data[0] = 'A';
            data[1] = 'T';
            data[2] = 'S';
            data[3] = m_channel;
            data[4] = size;
            std::memcpy(data + 5, std::ranges::cdata(range), size);
            uart_write_blocking(m_uart, data, size + 5);
        }

        /**
         *
         * @tparam PlainDataType - a numerical type or plain struct
         * @tag Completes on ZETAPLUS
         */
        template<typename PlainDataType>
        requires std::is_standard_layout_v<PlainDataType> && std::is_trivial_v<PlainDataType> && (sizeof(PlainDataType) < 64)
        void send(PlainDataType const &plain_data) noexcept {
            auto constexpr size = sizeof(PlainDataType);
            uint8_t data[size + 5];
            data[0] = 'A';
            data[1] = 'T';
            data[2] = 'S';
            data[3] = m_channel;
            data[4] = size;
            std::memcpy(data + 5, &plain_data, size);
            uart_write_blocking(m_uart, data, size + 5);
        }

        /**
         * @brief sets the operating mode of the ZETAPLUS
         * @param mode
         * @tag Completes on ZETAPLUS
         */
        void set_mode(zeta::mode_t mode) noexcept;

        /**
         * @brief set the UART baud rate
         */
        void set_uart_baud_rate(zeta::uart_baud_opt rate_option) noexcept;

        /**
         * @brief sets the rf baud rate and restarts the transceiver
         */
        void set_rf_baud_rate(zeta::rf_baud_opt rate_option) noexcept;

        /**
         * @brief set the output power
         * @param power_level - range 1 to 127
         * Will cause a restart, requiring rx config and mode to be set once again
         * @tag Completes on ZETAPLUS
         */
        void set_rf_output_power(uint8_t power_level) noexcept;

        /**
         * @param channel - TX transmission channel, must be less than 16
         */
        void set_output_channel(uint8_t channel) noexcept;

        /**
         * @brief reads directly from uart without waiting for ZETAPLUS packet header
         * @param dst - location to read to
         * @param bytes - number of bytes to read
         */
        void raw_read_to(void *dst, size_t bytes) noexcept;

        /**
         * @brief reads data received by the UART
         * @return the response from the ZETAPLUS
         *
         */
        response_t read() noexcept {
            // Wait for # which signifies the beginning of a packet
            while (uart_getc(m_uart) != '#') {}
            response_t res{};

            res.packet_type = (packet_type_t) uart_getc(m_uart);
            switch (res.packet_type) {
                case packet_type_t::TRANSMIT:
                case packet_type_t::RSSI:
                    uart_read_blocking(m_uart, (uint8_t *) &res.rssi, 1);
                    break;
                case packet_type_t::DATA:
                    res.read.length = uart_getc(m_uart);
                    res.read.rssi = uart_getc(m_uart);
                    uart_read_blocking(m_uart, (uint8_t *) res.read.data, res.read.length);
                    break;
                case packet_type_t::CONFIG:
                    uart_read_blocking(m_uart, (uint8_t *) &res.config, sizeof(device_config_t));
                    break;
                case packet_type_t::FIRMWARE:
                    res.firmware_str[0] = (int8_t)res.packet_type;
                    uart_read_blocking(m_uart, ((uint8_t*)res.firmware_str) + 1, 10);
                    res.firmware_str[11] = '\0';
                    break;
                default:
                    res.packet_type = packet_type_t::UNKNOWN;
            }
            return res;
        }

        /**
         * @brief request the ZETAPLUS firmware string
         * You will be able to receive the result from read()
         */
        void request_firmware() noexcept;

        /**
         * @brief request the Received Signal Strength Value
         * You will be able to receive the result from read()
         */
        void request_rssi() noexcept;

        /**
         * @brief request the ZETAPLUS config
         * You will be able to receive the result from read()
         */
        void request_device_config() noexcept;
    private:
        void restart() noexcept;
    private:
        uart_inst *m_uart;
        uint8_t m_channel{};
        uint m_pin_shutdown{};
        uint m_pin_rx{};
        uint m_pin_tx{};
        bool m_owns_uart;
    };


}
