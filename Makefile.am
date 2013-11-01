AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS = test_init
test_init_SOURCES = test/init_test.c compiler.c hash_table.c init.c macros.c object.c prims.c utilities.c vm.c
test_init_LDADD = -lgc -lgmp
test_init_CFLAGS = -std=c99 -D_GNU_SOURCE_
