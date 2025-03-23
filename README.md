# ESP8266 WiFi Module Driver for BeagleBone Green

This project provides a Linux-based driver for the ESP8266 WiFi module, specifically designed for the BeagleBone Green platform. It enables UART communication with the ESP8266 module to perform WiFi operations such as connecting to networks and making HTTP requests.

## Overview

The ESP8266 is a low-cost WiFi microchip that can be controlled via AT commands over a serial connection. This driver provides a C-based interface to:

- Initialize the ESP8266 module
- Connect to WiFi networks
- Send HTTP GET requests
- Parse responses from the module

The code is designed to run on a BeagleBone Green board running Linux, utilizing the UART interface for communication with the ESP8266 module.

## Features

- **UART Communication**: Configurable baudrate and port settings
- **AT Command Interface**: Structured approach to sending commands and parsing responses
- **Timeout Management**: Robust handling of command timeouts
- **WiFi Connection**: Simple API to connect to WiFi networks
- **HTTP Requests**: Support for making HTTP GET requests
- **Error Handling**: Comprehensive error reporting and recovery

## Hardware Requirements

- BeagleBone Green board
- ESP8266 WiFi module (ESP-01 or similar)
- Jumper wires for connections

## Wiring

Connect the ESP8266 module to the BeagleBone Green as follows:

| ESP8266 Pin | BeagleBone Green Pin |
|-------------|----------------------|
| VCC         | 3.3V                 |
| GND         | GND                  |
| RX          | UART1_TX (Pin #24)   |
| TX          | UART1_RX (Pin #26)   |
| CH_PD       | 3.3V                 |

## Software Dependencies

- GCC compiler
- Standard C libraries
- Linux kernel with UART support

## Project Structure

```
esp8266_bbg/
├── include/
│   ├── esp8266.h    - ESP8266 driver interface definitions
│   └── uart.h       - UART communication interface
├── src/
│   ├── esp8266.c    - ESP8266 driver implementation
│   ├── uart.c       - UART communication implementation
│   └── main.c       - Example application
└── Makefile         - Build configuration
```

## Building the Project

To build the project, simply run:

```bash
make
```

This will compile the source files and create an executable named `esp8266_app`.

To clean the build artifacts:

```bash
make clean
```

## Usage Example

The main.c file provides an example of how to use the ESP8266 driver:

```c
#include <stdio.h>
#include "uart.h"
#include "esp8266.h"

int main(int argc, char *argv[]) {
    // Initialize UART
    int uart_fd = init_uart("/dev/ttyS1", B115200);
    if (uart_fd < 0) {
        fprintf(stderr, "Failed to initialize UART (error code: %d)\n", uart_fd);
        return EXIT_FAILURE;
    }
    
    // WiFi credentials
    const char* ssid = "YourWiFiSSID";
    const char* password = "YourWiFiPassword";
    
    // Initialize ESP8266 and connect to WiFi
    printf("Initializing ESP8266 and connecting to WiFi...\n");
    if (!esp_init_and_connect(uart_fd, ssid, password)) {
        fprintf(stderr, "Failed to initialize ESP8266 or connect to WiFi\n");
        close(uart_fd);
        return EXIT_FAILURE;
    }
    
    printf("Connected to WiFi!\n");
    
    // Perform HTTP GET request
    printf("Sending HTTP GET request...\n");
    if (esp_http_get(uart_fd, "example.com", 80, "/")) {
        printf("HTTP request successful!\n");
        printf("Response: %s\n", response_buffer);
    } else {
        fprintf(stderr, "HTTP request failed\n");
    }
    
    // Clean up
    close(uart_fd);
    return 0;
}
```

## Customization

To use this driver in your own project:

1. Update the WiFi credentials in main.c
2. Modify the HTTP request parameters as needed
3. Integrate the ESP8266 functions into your application logic

## Troubleshooting

If you encounter issues:

1. **UART Connection Problems**: Verify the UART port name and baudrate settings
2. **ESP8266 Not Responding**: Check power connections and ensure the module is in working condition
3. **WiFi Connection Failures**: Verify SSID and password are correct
4. **HTTP Request Issues**: Check network connectivity and server availability

## Extending the Driver

The driver can be extended to support additional ESP8266 functionality:

- TCP/UDP socket communication
- Multiple connection management
- Access point mode
- Firmware updates
- Deep sleep mode control

## License

This project is released under the MIT License. See the LICENSE file for details.

## Acknowledgments

This project builds upon the UART communication examples from the Embedded Linux course and adapts the ESP8266 AT command interface for Linux-based systems.

## Contributing

Contributions to improve the driver are welcome. Please feel free to submit pull requests or open issues for bugs and feature requests.
