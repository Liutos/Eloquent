CC=gcc
CFLAGS=-Wall -g -std=c99 -D_GNU_SOURCE

all: test_vm

object.o: object.c object.h type.h
	$(CC) $(CFLAGS) -c $< -o $@

prims.o: prims.c object.h type.h
	$(CC) $(CFLAGS) -c $< -o $@

compiler.o: compiler.c object.h prims.h type.h utilities.h
	$(CC) $(CFLAGS) -c $< -o $@

vm.o: vm.c object.h type.h prims.h
	$(CC) $(CFLAGS) -c $< -o $@

utilities.o: utilities.c object.h type.h
	$(CC) $(CFLAGS) -c $< -o $@

vm_test.o: vm_test.c object.h type.h compiler.h vm.h prims.h
	$(CC) $(CFLAGS) -c $< -o $@

compiler_test.o: compiler_test.c object.h type.h prims.h compiler.h
	$(CC) $(CFLAGS) -c $< -o $@

test_vm: vm_test.o object.o prims.o compiler.o vm.o utilities.o
	$(CC) $(CFLAGS) $^ -o bin/$@

test_compiler: compiler_test.o object.o prims.o compiler.o vm.o utilities.o
	$(CC) $(CFLAGS) $^ -o bin/$@

.PHONY: clean

clean:
	rm -f *.o
