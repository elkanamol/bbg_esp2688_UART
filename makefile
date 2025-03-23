CC = gcc
CFLAGS = -Wall -O2 -Iinclude

all: esp8266_app

uart.o: src/uart.c include/uart.h
	$(CC) $(CFLAGS) -c src/uart.c -o uart.o

esp8266.o: src/esp8266.c include/esp8266.h
	$(CC) $(CFLAGS) -c src/esp8266.c -o esp8266.o

esp8266_app: src/main.c uart.o esp8266.o
	$(CC) $(CFLAGS) src/main.c uart.o esp8266.o -o esp8266_app

clean:
	rm -f esp8266_app *.o
