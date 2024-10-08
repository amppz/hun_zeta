#include "transceiver.h"


zeta::transceiver::transceiver(uart_inst *uart_inst, config_t const &cfg) noexcept
        : m_uart(uart_inst), m_owns_uart(true), m_pin_shutdown(cfg.pin_shutdown) {
    uart_init(m_uart, 19200);
    m_pin_shutdown = cfg.pin_shutdown;
    m_pin_rx = cfg.pin_rx;
    m_pin_tx = cfg.pin_tx;

    gpio_init(m_pin_shutdown);
    gpio_set_dir(m_pin_shutdown, GPIO_OUT);

    gpio_set_function(m_pin_rx, GPIO_FUNC_UART);
    gpio_set_function(m_pin_tx, GPIO_FUNC_UART);

    restart();
    configure_rx(cfg.receive_bytes, cfg.receive_channel);
    set_mode(zeta::mode_t::RX);
    set_uart_baud_rate(cfg.m_baud_rate);
}

zeta::transceiver::transceiver(uart_inst *uart, uart_baud_opt baud, uint pin_shutdown, uint8_t receive_bytes, uint8_t channel) noexcept
        : m_uart(uart), m_owns_uart(false), m_pin_shutdown(pin_shutdown) {
    uart_tx_wait_blocking(uart);
    uart_set_baudrate(uart, 19200);
    gpio_init(pin_shutdown);
    gpio_set_dir(pin_shutdown, GPIO_OUT);

    restart();

    configure_rx(receive_bytes, channel);
    set_mode(zeta::mode_t::RX);
    set_uart_baud_rate(baud);
}

zeta::transceiver::~transceiver() noexcept {
    gpio_deinit(m_pin_shutdown);
    if (!m_owns_uart)
        return;
    uart_deinit(m_uart);
    gpio_deinit(m_pin_rx);
    gpio_deinit(m_pin_tx);
}

void zeta::transceiver::set_mode(zeta::mode_t mode) noexcept {
    uint8_t data[4] = {'A', 'T', 'M', static_cast<uint8_t>(mode)};
    uart_write_blocking(m_uart, data, 4);
}

void zeta::transceiver::set_uart_baud_rate(zeta::uart_baud_opt rate_option) noexcept {
    uint8_t data[4] = {'A', 'T', 'H', static_cast<uint8_t>(rate_option)};

    uart_write_blocking(m_uart, data, 4);

    uart_tx_wait_blocking(m_uart);
    uart_set_baudrate(m_uart, uart_baud_opt_to_value(rate_option));
    sleep_ms(50);
}

void zeta::transceiver::set_rf_baud_rate(zeta::rf_baud_opt rate_option) noexcept {
    uint8_t data[4] = {'A', 'T', 'B', static_cast<uint8_t>(rate_option)};
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
    uart_tx_wait_blocking(m_uart);
    gpio_put(m_pin_shutdown, true);
    sleep_ms(50);
    gpio_put(m_pin_shutdown, false);
    sleep_ms(50);
}

void zeta::transceiver::send_from(const void *src, size_t byte_count) noexcept {
    uint8_t data[byte_count + 5];
    data[0] = 'A';
    data[1] = 'T';
    data[2] = 'S';
    data[3] = m_channel;
    data[4] = byte_count;
    std::memcpy(data + 5, src, byte_count);
    uart_write_blocking(m_uart, data, byte_count + 5);
}

void zeta::transceiver::request_firmware() noexcept {
    uint8_t data[] = {'A', 'T', 'V'};
    uart_write_blocking(m_uart, data, 3);
}

void zeta::transceiver::request_rssi() noexcept {
    uint8_t data[] = {'A', 'T', 'Q'};
    uart_write_blocking(m_uart, data, 3);
}


void zeta::transceiver::raw_read_to(void *dst, size_t bytes) noexcept {
    uart_read_blocking(m_uart, (uint8_t *) dst, bytes);
}

void zeta::transceiver::configure_rx(uint8_t byte_count, uint8_t receive_channel) noexcept {
    if (byte_count > 4)
        byte_count = 64;
    uint8_t data[5] = {'A', 'T', 'R', receive_channel, byte_count};
    uart_write_blocking(m_uart, data, 5);
}

void zeta::transceiver::request_device_config() noexcept {
    uint8_t data[3] = {'A', 'T', '?'};
    uart_write_blocking(m_uart, data, 3);
}


