CC=gcc
CFLAGS=-Wall -g -std=c99 -D_GNU_SOURCE

all: test_vm

object.o: object.c object.h type.h
	$(CC) $(CFLAGS) -c $< -o $@

prims.o: prims.c object.h type.h
	$(CC) $(CFLAGS) -c $< -o $@

compiler.o: compiler.c object.h type.h prims.h
	$(CC) $(CFLAGS) -c $< -o $@

vm.o: vm.c object.h type.h prims.h
	$(CC) $(CFLAGS) -c $< -o $@

vm_test.o: vm_test.c object.h type.h compiler.h vm.h
	$(CC) $(CFLAGS) -c $< -o $@

test_vm: vm_test.o object.o prims.o compiler.o vm.o
	$(CC) $(CFLAGS) $^ -o bin/$@

.PHONY: clean

clean:
	rm -f *.o
	if [ -f test_vm ]; then rm test_vm; fi
