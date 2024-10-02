all: gpio

LINK=-Wall -Wno-char-subscripts 
CC=gcc

gpio: gpio.c
	$(CC) $(LINK) $^ -lm -o $@ 

clean:
	rm gpio
	
install:
	chmod 777 brltty
