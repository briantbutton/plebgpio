#!/usr/bin/make -f

LINK=-Wall -Wno-char-subscripts 
CC=gcc

all: plebgpio

plebgpio: plebgpio.c
	$(CC) $(LINK) $^ -lm -o $@ 

clean:
	rm plebgpio
	
install: 
	install plebgpio $(DESTDIR)/usr/bin/plebgpio

