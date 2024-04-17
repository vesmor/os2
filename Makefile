CC = gcc
CFLAGS = -Wall -Wextra -pthread

chash: chash.o hashdb.o rwlocks.o
    $(CC) $(CFLAGS) -o chash chash.o hashdb.o rwlocks.o

chash.o: chash.c hashdb.h rwlocks.h common.h common_threads.h
    $(CC) $(CFLAGS) -c chash.c

hashdb.o: hashdb.c hashdb.h common.h
    $(CC) $(CFLAGS) -c hashdb.c

rwlocks.o: rwlocks.c rwlocks.h common.h
    $(CC) $(CFLAGS) -c rwlocks.c

clean:
    rm -f chash *.o

.PHONY: clean
