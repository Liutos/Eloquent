CC = gcc
CFLAGS = -Wall -g -pg
SRCS = $(shell find . -maxdepth 1 -name '*.c' -not -name '*main.c')
OBJS = $(patsubst %.c, %.o, $(SRCS))
ELOI = eloi
ELOC = eloc

all: $(ELOI) $(ELOC)

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend

include .depend

# objects

utils/hash_table.o: utils/hash_table.c utils/hash_table.h

utils/string.o: utils/string.c utils/string.h

utils/vector.o: utils/vector.c utils/vector.h

# executables

$(ELOI): main.o $(OBJS) utils/hash_table.o utils/string.o utils/vector.o
	$(CC) $(CFLAGS) -o $@ $^

$(ELOC): cv_main.o $(OBJS) utils/hash_table.o utils/string.o utils/vector.o
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	if [ -f $(ELOI) ]; then rm -vf $(ELOI); fi
	if [ -f $(ELOC) ]; then rm -vf $(ELOC); fi
	find . -name '*.o' -exec rm -rvf {} \;