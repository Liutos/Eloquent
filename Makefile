all: eloi

# objects

lexer.o: lexer.c lexer.h utils/string.h
	gcc -Wall -g -c -o $@ $<

ast.o: ast.c ast.h utils/string.h
	gcc -Wall -g -c -o $@ $<

env.o: env.c env.h utils/hash_table.h
	gcc -Wall -g -c -o $@ $<

interp.o: interp.c interp.h utils/hash_table.h
	gcc -Wall -g -c -o $@ $<

parser.o: parser.c ast.h lexer.h parser.h
	gcc -Wall -g -c -o $@ $<

main.o: main.c ast.h lexer.h parser.h
	gcc -Wall -g -c -o $@ $<

utils/hash_table.o: utils/hash_table.c utils/hash_table.h
	gcc -Wall -g -c -o $@ $<

utils/string.o: utils/string.c utils/string.h
	gcc -Wall -g -c -o $@ $<

value.o: value.c value.h utils/string.h
	gcc -Wall -g -c -o $@ $<

# executables

eloi: ast.o env.o lexer.o interp.o main.o parser.o utils/hash_table.o utils/string.o value.o
	gcc -Wall -g -o $@ $^