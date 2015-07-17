all: eloi

# objects

lexer.o: lexer.c lexer.h utils/string.h
	gcc -Wall -g -c -o $@ $<

ast.o: ast.c ast.h utils/string.h
	gcc -Wall -g -c -o $@ $<

parser.o: parser.c ast.h lexer.h parser.h
	gcc -Wall -g -c -o $@ $<

main.o: main.c ast.h lexer.h parser.h
	gcc -Wall -g -c -o $@ $<

utils/string.o: utils/string.c utils/string.h
	gcc -Wall -g -c -o $@ $<

# executables

eloi: ast.o lexer.o main.o parser.o utils/string.o
	gcc -Wall -g -o $@ $^