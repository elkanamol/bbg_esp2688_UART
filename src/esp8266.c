#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include "../include/esp8266.h" // Use relative path if needed

// Global response buffer
char response_buffer[2048];

typedef enum
{
    ESP_STATE_RESET,
    ESP_STATE_INIT,
    ESP_STATE_CONNECT_AP,
    ESP_STATE_CONNECTED,
    ESP_STATE_TCP_CONNECT,
    ESP_STATE_TCP_CONNECTED,
    ESP_STATE_HTTP_SEND,
    ESP_STATE_HTTP_RESPONSE,
    ESP_STATE_TCP_CLOSE,
    ESP_STATE_ERROR
} ESP_State;

ESP_State esp_state = ESP_STATE_RESET;
char response_buffer[2048];

bool esp_init_and_connect(int uart_fd, const char *ssid, const char *password)
{
    ESP_Status status;

    // State machine for initialization and connection
    while (esp_state != ESP_STATE_CONNECTED && esp_state != ESP_STATE_ERROR)
    {
        switch (esp_state)
        {
        case ESP_STATE_RESET:
            // Reset module
            status = esp_send_command_wait_response(uart_fd, "AT+RST",
                                                    response_buffer, sizeof(response_buffer),
                                                    "ready", 5000);
            if (status == ESP_OK)
            {
                esp_state = ESP_STATE_INIT;
                // Small delay after reset
                usleep(1000000); // 1 second
            }
            else
            {
                esp_state = ESP_STATE_ERROR;
            }
            break;

        case ESP_STATE_INIT:
            // Test AT command
            status = esp_send_command_wait_response(uart_fd, "AT",
                                                    response_buffer, sizeof(response_buffer),
                                                    "OK", 1000);
            if (status == ESP_OK)
            {
                // Set station mode
                status = esp_send_command_wait_response(uart_fd, "AT+CWMODE=1",
                                                        response_buffer, sizeof(response_buffer),
                                                        "OK", 1000);
                if (status == ESP_OK)
                {
                    esp_state = ESP_STATE_CONNECT_AP;
                }
                else
                {
                    esp_state = ESP_STATE_ERROR;
                }
            }
            else
            {
                esp_state = ESP_STATE_ERROR;
            }
            break;

        case ESP_STATE_CONNECT_AP:
            // Connect to AP
            {
                char connect_cmd[128];
                snprintf(connect_cmd, sizeof(connect_cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);

                status = esp_send_command_wait_response(uart_fd, connect_cmd,
                                                        response_buffer, sizeof(response_buffer),
                                                        "WIFI CONNECTED", 20000);

                if (status == ESP_OK)
                {
                    // Wait for IP address
                    status = esp_send_command_wait_response(uart_fd, "",
                                                            response_buffer, sizeof(response_buffer),
                                                            "WIFI GOT IP", 10000);
                    if (status == ESP_OK)
                    {
                        esp_state = ESP_STATE_CONNECTED;
                    }
                    else
                    {
                        esp_state = ESP_STATE_ERROR;
                    }
                }
                else
                {
                    esp_state = ESP_STATE_ERROR;
                }
            }
            break;

        default:
            esp_state = ESP_STATE_ERROR;
            break;
        }
    }

    return (esp_state == ESP_STATE_CONNECTED);
}


ESP_Status esp_send_command_wait_response(int uart_fd, 
                                         const char* command,
                                         char* response_buffer, 
                                         size_t buffer_size,
                                         const char* expected_response, 
                                         uint32_t timeout_ms) {
    // Clear any pending data
    tcflush(uart_fd, TCIOFLUSH);
    
    // Send command
    char cmd_buffer[256];
    snprintf(cmd_buffer, sizeof(cmd_buffer), "%s\r\n", command);
    if (write(uart_fd, cmd_buffer, strlen(cmd_buffer)) < 0) {
        perror("Error writing to UART");
        return ESP_ERROR;
    }
    
    // Wait for response with timeout
    fd_set readfds;
    struct timeval tv;
    time_t start_time = time(NULL);
    int total_bytes = 0;
    
    // Continue reading until timeout or expected response found
    while (1) {
        // Check if we've exceeded timeout
        if ((time(NULL) - start_time) * 1000 >= timeout_ms) {
            return ESP_TIMEOUT;
        }
        
        FD_ZERO(&readfds);
        FD_SET(uart_fd, &readfds);
        
        // Set remaining timeout
        uint32_t elapsed_ms = (time(NULL) - start_time) * 1000;
        uint32_t remaining_ms = (elapsed_ms < timeout_ms) ? (timeout_ms - elapsed_ms) : 0;
        
        tv.tv_sec = remaining_ms / 1000;
        tv.tv_usec = (remaining_ms % 1000) * 1000;
        
        int select_result = select(uart_fd + 1, &readfds, NULL, NULL, &tv);
        
        if (select_result > 0) {
            // Data is available to read
            char temp_buffer[64] = {0};
            int bytes_read = read(uart_fd, temp_buffer, sizeof(temp_buffer) - 1);
            
            if (bytes_read > 0) {
                // Ensure we don't overflow the buffer
                if (total_bytes + bytes_read >= buffer_size) {
                    bytes_read = buffer_size - total_bytes - 1;
                }
                
                // Append to response buffer
                memcpy(response_buffer + total_bytes, temp_buffer, bytes_read);
                total_bytes += bytes_read;
                response_buffer[total_bytes] = '\0';
                
                // Check if expected response is found
                if (expected_response && strstr(response_buffer, expected_response)) {
                    return ESP_OK;
                }
                
                // Check for error
                if (strstr(response_buffer, "ERROR")) {
                    return ESP_ERROR;
                }
            }
        } else if (select_result == 0) {
            // Timeout on select
            continue;
        } else {
            // Error on select
            return ESP_ERROR;
        }
    }
}

/**
 * Extract a substring between two delimiter strings
 */
bool esp_extract_substring(const char *source, const char *start_delim,
                           const char *end_delim, char *output, size_t output_size)
{
    char *start_pos = strstr(source, start_delim);
    if (!start_pos)
    {
        return false;
    }

    start_pos += strlen(start_delim);
    char *end_pos = strstr(start_pos, end_delim);

    if (!end_pos)
    {
        return false;
    }

    size_t length = end_pos - start_pos;
    if (length >= output_size)
    {
        length = output_size - 1;
    }

    strncpy(output, start_pos, length);
    output[length] = '\0';

    return true;
}

bool esp_http_get(int uart_fd, const char *host, int port, const char *path)
{
    ESP_Status status;
    char cmd[512];

    // Start TCP connection
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%d", host, port);
    status = esp_send_command_wait_response(uart_fd, cmd,
                                            response_buffer, sizeof(response_buffer),
                                            "CONNECT", 10000);
    if (status != ESP_OK)
    {
        return false;
    }

    // Prepare HTTP GET request
    char request[512];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
             path, host);

    // Send request length
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d", (int)strlen(request));
    status = esp_send_command_wait_response(uart_fd, cmd,
                                            response_buffer, sizeof(response_buffer),
                                            ">", 5000);
    if (status != ESP_OK)
    {
        return false;
    }

    // Send the actual request
    status = esp_send_command_wait_response(uart_fd, request,
                                            response_buffer, sizeof(response_buffer),
                                            "+IPD", 10000);
    if (status != ESP_OK)
    {
        return false;
    }

    // Wait for complete response
    // This is simplified - you might need to handle multiple +IPD packets
    status = esp_send_command_wait_response(uart_fd, "",
                                            response_buffer, sizeof(response_buffer),
                                            "CLOSED", 15000);

    return (status == ESP_OK);
}
