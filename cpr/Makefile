CC=gcc
CFLAGS=-std=gnu99 -Wall -Werror -O0 -g -I.
LIBS=-laio

all: clean cpr

cpr: cpr.c aio_manager.c
	$(CC) $(CFLAGS) cpr.c aio_manager.c -o cpr $(LIBS)

cpr_old: cpr_old.c
	$(CC) $(CFLAGS) cpr_old.c -o cpr_old $(LIBS)

clean:
	rm -f cpr cpr_old
