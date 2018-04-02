# parashell Makefile
# this will compile the parashell (output) and pin (input)
OWNER=root
GROUP=root
CFLAGS= -O
CC=gcc
BINDIR=/usr/local/bin

all: parashell

parashell:
	$(CC) $(CFLAGS) src/parashell.c -o bin/parashell
	$(CC) $(CFLAGS) src/pin.c -o bin/pin
install:
	cp bin/parashell $(BINDIR)
	cp bin/pin $(BINDIR)
clean:
	-rm -f core
