#include "transceiver.h"


zeta::transceiver::transceiver(uart_inst *uart_inst, config_t const& cfg) noexcept
        : m_uart(uart_inst) {
    uart_init(m_uart, uart_baud_opt_to_value(cfg.m_baud_rate));
    m_pin_shutdown = cfg.pin_shutdown;
    m_pin_rx = cfg.pin_rx;
    m_pin_tx = cfg.pin_tx;

    gpio_init(m_pin_shutdown);
    gpio_set_dir(m_pin_shutdown, GPIO_OUT);

    gpio_set_function(m_pin_rx, GPIO_FUNC_UART);
    gpio_set_function(m_pin_tx, GPIO_FUNC_UART);

    restart();
    configure_rx(cfg.receive_bytes, cfg.receive_channel);
    set_mode(zeta::mode_t::READY);
}

zeta::transceiver::~transceiver() noexcept {
    uart_deinit(m_uart);
    gpio_deinit(m_pin_shutdown);
    gpio_deinit(m_pin_rx);
    gpio_deinit(m_pin_tx);
}

void zeta::transceiver::set_mode(zeta::mode_t mode) noexcept {
    uint8_t data[4] = {'A', 'T', 'M', static_cast<uint8_t>(mode)};
    uart_write_blocking(m_uart, data, 4);
}

void zeta::transceiver::set_uart_baud_rate(zeta::uart_baud_opt rate) noexcept {
    uint8_t data[4] = {'A', 'T', 'H', static_cast<uint8_t>(rate)};
    uart_write_blocking(m_uart, data, 4);
    uart_set_baudrate(m_uart, uart_baud_opt_to_value(rate));
}

void zeta::transceiver::set_rf_baud_rate(zeta::rf_baud_opt rate) noexcept {
    uint8_t data[4] = {'A', 'T', 'B', static_cast<uint8_t>(rate)};
    uart_write_blocking(m_uart, data, 4);
    restart();
}

void zeta::transceiver::set_rf_output_power(uint8_t power_level) noexcept {
    uint8_t data[4] = {'A', 'T', 'P', power_level};
    uart_write_blocking(m_uart, data, 4);
}

void zeta::transceiver::set_output_channel(uint8_t channel) noexcept {
    if (channel >= 16)
        return;
    m_channel = channel;
}

void zeta::transceiver::restart() noexcept {
    sleep_ms(20);
    gpio_put(m_pin_shutdown, true);
    sleep_ms(50);
    gpio_put(m_pin_shutdown, false);
    sleep_ms(50);
}

void zeta::transceiver::send_from(void *begin, size_t bytes) noexcept {
    uint8_t data[bytes + 5];
    data[0] = 'A';
    data[1] = 'T';
    data[2] = 'S';
    data[3] = m_channel;
    data[4] = bytes;
    std::memcpy(data + 5, begin, bytes);
    uart_write_blocking(m_uart, data, bytes + 5);
}

void zeta::transceiver::request_firmware() noexcept {
    uint8_t data[] = {'A', 'T', 'V'};
    uart_write_blocking(m_uart, data, 3);
}

void zeta::transceiver::request_rssi() noexcept {
    uint8_t data[] = {'A', 'T', 'Q'};
    uart_write_blocking(m_uart, data, 3);
}


void zeta::transceiver::raw_read_to(void *dst, int bytes) noexcept {
    uart_read_blocking(m_uart, (uint8_t *) dst, bytes);
}

void zeta::transceiver::configure_rx(uint8_t byte_count, uint8_t receive_channel) {
    assert(byte_count < 64);
    uint8_t data[5] = {'A', 'T', 'R', receive_channel, byte_count};
    uart_write_blocking(m_uart, data, 5);
}
