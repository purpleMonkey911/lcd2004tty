/**
 * @file uart_inst.c
 * 
 * @brief UART instance for the app
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#include "app_config.h"
#include "mhal_gpio.h"

static Uart uart_inst = 0;

void Uart_Init(void) {
    UartConfig uart_config = {
        .gpio = {
            .tx.port = UART_PORT,
            .tx.pin  = UART_TX_PIN,
            .tx_config = {
                .mode = GPIO_MODE_ALT_PP,
                .pull = GPIO_PULL_UP,
                .speed = GPIO_SPEED_HI,
                .alt_func = GPIO_AF_7_USART3,
            },
            .rx.port = UART_PORT,
            .rx.pin = UART_RX_PIN,
            .rx_config = {
                .mode = GPIO_MODE_ALT_PP,
                .pull = GPIO_PULL_UP,
                .speed = GPIO_SPEED_HI,
                .alt_func = GPIO_AF_7_USART3,
            },
        },
        .index = UART_INDEX,
        .baud = UART_BAUD,
        .data_bits = UART_DATA_BITS,
        .stop_bits = UART_STOP_BITS,
        .parity_bits = UART_PARITY,
        .xfer = UART_XFER_FULL_DUPLEX,
        .flow_ctrl = UART_HW_FLOW_CTRL_NONE,
        .ring_buffer_size = UART_RING_BUFFER_SIZE_BYTES
    };
    mHalUart_Init(&uart_inst, &uart_config);
}

Uart Uart_GetInstance(void) {
    return uart_inst;
}
