#include "transceiver.h"


zeta::transceiver::transceiver(uart_inst *uart_inst)
        : m_uart(uart_inst) {
}

zeta::transceiver::~transceiver() {
    uart_deinit(m_uart);
    gpio_deinit(m_pin_shutdown);
    gpio_deinit(m_pin_rx);
    gpio_deinit(m_pin_tx);
}

void zeta::transceiver::start(uart_baud_opt baud_rate_opt, uint pin_shutdown, uint pin_rx, uint pin_tx) {
    uart_init(m_uart, uart_baud_opt_to_value(baud_rate_opt));
    gpio_init(pin_shutdown);
    gpio_init(pin_rx);
    gpio_init(pin_tx);
    gpio_set_dir(pin_shutdown, GPIO_OUT);
    gpio_set_function(pin_rx, GPIO_FUNC_UART);
    gpio_set_function(pin_tx, GPIO_FUNC_UART);
    m_pin_shutdown = pin_shutdown;
    m_pin_rx = pin_rx;
    m_pin_tx = pin_tx;
    restart();
}

void zeta::transceiver::set_mode(zeta::mode_t mode) {
    uint8_t data[4] = {'A', 'T', 'M', static_cast<uint8_t>(mode)};
    uart_write_blocking(m_uart, data, 4);
}

void zeta::transceiver::set_uart_baud_rate(zeta::uart_baud_opt rate) {
    uint8_t data[4] = {'A', 'T', 'H', static_cast<uint8_t>(rate)};
    uart_write_blocking(m_uart, data, 4);
    uart_set_baudrate(m_uart, uart_baud_opt_to_value(rate));
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

void zeta::transceiver::restart() {
    gpio_put(m_pin_shutdown, true);
    sleep_ms(15);
    gpio_put(m_pin_shutdown, false);
}

void zeta::transceiver::send_from(void *begin, size_t bytes) {
    uint8_t data[bytes + 5];
    data[0] = 'A';
    data[1] = 'T';
    data[2] = 'S';
    data[3] = m_channel;
    data[4] = bytes;
    std::memcpy(data + 5, begin, bytes);
    uart_write_blocking(m_uart, data, bytes + 5);
}

void zeta::transceiver::request_firmware() {
    uint8_t data[] = {'A', 'T', 'V'};
    uart_write_blocking(m_uart, data, 3);
}

void zeta::transceiver::request_rssi() {
    uint8_t data[] = {'A', 'T', 'Q'};
    uart_write_blocking(m_uart, data, 3);
}


void zeta::transceiver::raw_read_to(void *dst, int bytes) {
    uart_read_blocking(m_uart, (uint8_t *) dst, bytes);
}
