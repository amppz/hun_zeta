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

    template<typename T> requires std::is_trivial_v<T> && std::is_standard_layout_v<T>
    struct response_t {
        packet_type_t packet_type{packet_type_t::UNKNOWN};
        union {
            uint8_t rssi;
            device_config_t config;
            int8_t firmware_str[12];
            union {
                struct {
                    uint8_t length{};
                    uint8_t rssi{};
                    union {
                        uint8_t data[64];
                        T value{};
                    };
                };
            } read{};

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

    class transceiver {
    public:
        explicit transceiver(uart_inst *uart_inst, config_t const& config) noexcept;

        ~transceiver() noexcept;

        /**
 * Configure RX mode
 * @param byte_count - max number of bytes reads should receive, max is 64
 * @param receive_channel - channel that reads will arrive on
 */
        void configure_rx(uint8_t byte_count, uint8_t receive_channel);

        void send_from(void *begin, size_t bytes) noexcept;

        // Data in range must be less than 64 bytes
        void send_from(std::ranges::contiguous_range auto r) noexcept {
            auto const size = std::ranges::size(r);
            uint8_t data[size + 5];
            data[0] = 'A';
            data[1] = 'T';
            data[2] = 'S';
            data[3] = m_channel;
            data[4] = size;
            std::memcpy(data + 5, std::ranges::cdata(r), size);
            uart_write_blocking(m_uart, data, size + 5);
        }

        // Has to be plain, and less than 64 bytes
        template<typename T>
        requires std::is_standard_layout_v<T> && std::is_trivial_v<T> && (sizeof(T) < 64)
        void send(T const &plain_data) noexcept {
            auto constexpr size = sizeof(T);
            uint8_t data[size + 5];
            data[0] = 'A';
            data[1] = 'T';
            data[2] = 'S';
            data[3] = m_channel;
            data[4] = size;
            std::memcpy(data + 5, &plain_data, size);
            uart_write_blocking(m_uart, data, size + 5);
        }

        void set_mode(zeta::mode_t mode) noexcept;

        /**
         * \brief
         * \param rate use the enum
         * \return true on success
         */
        void set_uart_baud_rate(zeta::uart_baud_opt rate) noexcept;

        /**
         * \brief sets the rf baud rate and restarts the transceiver
         * \return
         */
        void set_rf_baud_rate(zeta::rf_baud_opt rate) noexcept;

        /**
         * \brief set the output power
         * @param power_level
         * Will cause a restart, requiring rx config and mode to be set once again
         */
        void set_rf_output_power(uint8_t power_level) noexcept;

        /**
         * \param channel must be less than 16
         */
        void set_output_channel(uint8_t channel) noexcept;

        /**
         * @param dst location to read to
         * @param bytes number of bytes to read
         */
        void raw_read_to(void *dst, int bytes) noexcept;

        template<typename T>
        requires std::is_trivial_v<T> && std::is_standard_layout_v<T>
        response_t<T> read() noexcept {
            // Wait for # which signifies the beginning of a packet
            while (uart_getc(m_uart) != '#') {}
            response_t<T> res{};

            res.packet_type = (packet_type_t) uart_getc(m_uart);
            switch (res.packet_type) {
                case packet_type_t::TRANSMIT:
                case packet_type_t::RSSI:
                    uart_read_blocking(m_uart, (uint8_t *) &res.rssi, 1);
                    break;
                case packet_type_t::READ:
                    res.read.length = uart_getc(m_uart);
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

        void request_firmware() noexcept;

        void request_rssi() noexcept;

        void request_device_config() noexcept;

    private:
        void restart() noexcept;


    private:
        uart_inst *m_uart;
        uint8_t m_channel{};
        uint m_pin_shutdown{};
        uint m_pin_rx{};
        uint m_pin_tx{};
        constexpr static auto max_size = 64 + 5;
    };


}
