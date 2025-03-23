#ifndef ESP8266_H
#define ESP8266_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Response status
typedef enum
{
    ESP_OK,
    ESP_TIMEOUT,
    ESP_ERROR,
    ESP_BUSY
} ESP_Status;

typedef struct
{
    const char *command;  // Command string
    const char *response; // Expected response
    uint32_t timeout;     // Timeout in milliseconds
} ESP_ATCommand;

// Function declarations
ESP_Status esp_send_command_wait_response(int uart_fd,
                                          const char *command,
                                          char *response_buffer,
                                          size_t buffer_size,
                                          const char *expected_response,
                                          uint32_t timeout_ms);

bool esp_extract_substring(const char *source, const char *start_delim,
                           const char *end_delim, char *output, size_t output_size);

bool esp_init_and_connect(int uart_fd, const char *ssid, const char *password);

bool esp_http_get(int uart_fd, const char *host, int port, const char *path);

// Make response buffer accessible
extern char response_buffer[2048];

#endif // ESP8266_H
