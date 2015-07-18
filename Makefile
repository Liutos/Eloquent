CC = gcc
CFLAGS = -Wall -g
SRCS = $(shell find . -maxdepth 1 -name '*.c')
OBJS = $(patsubst %.c, %.o, $(SRCS))
ELOI = eloi

all: $(ELOI)

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend

include .depend

# objects

utils/hash_table.o: utils/hash_table.c utils/hash_table.h

utils/string.o: utils/string.c utils/string.h

# executables

$(ELOI): $(OBJS) utils/hash_table.o utils/string.o
	gcc -Wall -g -o $@ $^

.PHONY: clean

clean:
	if [ -f $(ELOI) ]; then rm -vf $(ELOI); fi
	find . -name '*.o' -exec rm -rvf {} \;