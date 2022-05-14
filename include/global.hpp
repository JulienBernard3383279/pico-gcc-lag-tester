#ifndef __GLOBAL_HPP
#define __GLOBAL_HPP

#include "pico/stdlib.h"

const int us = 125;

#define LED_PIN 25


#pragma region logs

#define USE_UART0 0
#define USE_UART1 0

inline void initialize_uart() {
    #if USE_UART0
    // Initialise UART 0
    uart_init(uart0, 115200);
 
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
    #endif
    #if USE_UART1
    // Initialise UART 0
    uart_init(uart1, 115200);
 
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(8, GPIO_FUNC_UART);
    gpio_set_function(9, GPIO_FUNC_UART);
    #endif

}
inline void log_uart0(const char* str) {
    #if USE_UART0
    uart_puts(uart0, str);
    #endif
    #if USE_UART1
    uart_puts(uart1, str);
    #endif
}
inline void log_uart0_int(int i) {
    #if USE_UART0
    char str[16];
    sprintf(str, "%d", i);
    uart_puts(uart0, str);
    #endif
    #if USE_UART1
    char str[16];
    sprintf(str, "%d", i);
    uart_puts(uart1, str);
    #endif
}

inline void log_uart0_array(uint8_t *ptr, uint16_t len) {
    #if USE_UART0
    log_uart0("array len=");log_uart0_int(len);log_uart0("\n");
    for (int i = 0; i<len; i++) {
        log_uart0_int(ptr[i]); log_uart0(" ");
    }
    log_uart0("\n");
    #endif
    #if USE_UART1
    log_uart0("array len=");log_uart0_int(len);log_uart0("\n");
    for (int i = 0; i<len; i++) {
        log_uart0_int(ptr[i]); log_uart0(" ");
    }
    log_uart0("\n");
    #endif
}

#pragma endregion

#endif