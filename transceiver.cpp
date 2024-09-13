#include "transceiver.h"

static constexpr uint16_t to_value(zeta::uart_baud_opt baud) {
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


zeta::transceiver::transceiver(uart_inst* uart_inst)
: m_uart(uart_inst) {
}

zeta::transceiver::~transceiver() {
    uart_deinit(m_uart);
    gpio_deinit(m_pin_shutdown);
}

void zeta::transceiver::start(uart_baud_opt baud_rate_opt, uint pin_shutdown) {
    uart_init(m_uart, to_value(baud_rate_opt));
    gpio_init(pin_shutdown);
    gpio_set_dir(pin_shutdown, GPIO_OUT);
    restart();
    m_pin_shutdown = pin_shutdown;
}

void zeta::transceiver::set_mode(zeta::mode_t mode) {
    uint8_t data[4] = {'A', 'T', 'M', static_cast<uint8_t>(mode)};
    uart_write_blocking(m_uart, data, 4);
}

void zeta::transceiver::set_uart_baud_rate(zeta::uart_baud_opt rate) {
    uint8_t data[4] = {'A', 'T', 'H', static_cast<uint8_t>(rate)};
    uart_write_blocking(m_uart, data, 4);
    uart_set_baudrate(m_uart, to_value(rate));
}

void zeta::transceiver::set_rf_baud_rate(zeta::rf_baud_opt rate) {
    uint8_t data[4] = {'A', 'T', 'B', static_cast<uint8_t>(rate)};
    uart_write_blocking(m_uart, data, 4);
}

void zeta::transceiver::set_rf_output_power(uint8_t power_level) {
    uint8_t data[4] = {'A', 'T', 'P', power_level};
    uart_write_blocking(m_uart, data, 4);
}

void zeta::transceiver::set_transmission_channel(uint8_t channel) {
    if (channel >= 16)
        return;
    m_channel = channel;
}

void zeta::transceiver::read_to(uint8_t* data, size_t size) {
    uart_read_blocking(m_uart, data, size);
}

void zeta::transceiver::restart() {
    gpio_put(m_pin_shutdown, true);
    sleep_ms(15);
    gpio_put(m_pin_shutdown, false);
}
