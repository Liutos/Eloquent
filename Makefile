CC=gcc
CFLAGS=-Wall -g -std=c99 -D_GNU_SOURCE

prototype.o: prototype.c

prototype: prototype.o
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f *.o
	if [ -f prototype ]; then rm prototype; fi
