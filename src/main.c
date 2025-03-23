#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

#include "../include/uart.h"    // Use relative path
#include "../include/esp8266.h" // Use relative path

// Rest of the file remains the same

int main(int argc, char *argv[])
{
    // Initialize UART
    int uart_fd = init_uart("/dev/zero", B115200);
    if (uart_fd < 0)
    {
        fprintf(stderr, "Failed to initialize UART (error code: %d)\n", uart_fd);
        return EXIT_FAILURE;
    }

    // WiFi credentials
    const char *ssid = "YourWiFiSSID";
    const char *password = "YourWiFiPassword";

    // Initialize ESP8266 and connect to WiFi
    printf("Initializing ESP8266 and connecting to WiFi...\n");
    if (!esp_init_and_connect(uart_fd, ssid, password))
    {
        fprintf(stderr, "Failed to initialize ESP8266 or connect to WiFi\n");
        close(uart_fd);
        return EXIT_FAILURE;
    }

    printf("Connected to WiFi!\n");

    // Perform HTTP GET request
    printf("Sending HTTP GET request...\n");
    if (esp_http_get(uart_fd, "example.com", 80, "/"))
    {
        printf("HTTP request successful!\n");
        printf("Response: %s\n", response_buffer);
    }
    else
    {
        fprintf(stderr, "HTTP request failed\n");
    }

    // Clean up
    close(uart_fd);
    return 0;
}
