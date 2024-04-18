CC=gcc
CFLAGS=-Wall -pthread

all: chash

chash: chash.o hashdb.o rwlocks.o
	$(CC) $(CFLAGS) -o chash chash.o hashdb.o rwlocks.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o chash

.PHONY: all clean
