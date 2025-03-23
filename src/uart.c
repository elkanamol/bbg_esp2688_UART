#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

#include "../include/uart.h"

/* Configuration constants */
#define UART_MIN_CHARS       1   /* Minimum characters for read to return */
#define UART_TIMEOUT_DECISEC 5   /* Read timeout in deciseconds */

int init_uart(const char* port, speed_t baudrate)
{
    if (!port) {
        fprintf(stderr, "Error: NULL port name provided\n");
        return UART_ERROR_OPEN;
    }

    int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1) {
        perror("Error opening serial port");
        return UART_ERROR_OPEN;
    }

    struct termios options;

    if (tcgetattr(fd, &options) < 0) {
        perror("Error getting current port settings");
        close(fd);
        return UART_ERROR_GETATTR;
    }

    /* Set baudrate */
    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);

    /* 8N1 configuration */
    options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;
    options.c_iflag = IGNBRK;
    options.c_oflag = 0;
    options.c_lflag = 0;
    
    /* Enable receiver, local mode */
    options.c_cflag |= CLOCAL | CREAD;

    /* Read behavior: wait for at least 1 char, timeout after 0.5 seconds */
    options.c_cc[VMIN] = UART_MIN_CHARS;
    options.c_cc[VTIME] = UART_TIMEOUT_DECISEC;

    /* Disable software flow control */
    options.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* Disable parity */
    options.c_cflag &= ~(PARENB | PARODD);

    if (tcsetattr(fd, TCSANOW, &options) < 0) {
        perror("Error applying port settings");
        close(fd);
        return UART_ERROR_SETATTR;
    }

    /* Clear input and output buffers */
    tcflush(fd, TCIOFLUSH);
    return fd;
}
