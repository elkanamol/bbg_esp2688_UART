#ifndef UART_H
#define UART_H

#include <termios.h> // For speed_t type

/**
 * @brief Initialize and configure a UART port
 *
 * @param port Path to the serial port device (e.g., "/dev/ttyS1")
 * @param baudrate Baud rate to set (use standard values like B9600, B115200)
 * @return int File descriptor on success, negative error code on failure
 */
int init_uart(const char *port, speed_t baudrate);

/**
 * Error codes returned by init_uart
 */
#define UART_ERROR_OPEN -1
#define UART_ERROR_GETATTR -2
#define UART_ERROR_SETATTR -3

#endif // UART_H
