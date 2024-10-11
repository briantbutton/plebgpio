#!/usr/bin/make -f

LINK=-Wall -Wno-char-subscripts 
CC=gcc

all: plebgpio

plebgpio: plebgpio.c
	$(CC) $(LINK) $^ -lm -o $@ 

clean:
	rm plebgpio
	
install:
	sudo mv plebgpio /bin
	sudo chown root:service /bin/plebgpio
	sudo chmod 4755 /bin/plebgpio
