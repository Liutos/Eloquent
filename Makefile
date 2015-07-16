lexer.o: lexer.c lexer.h utils/string.h
	gcc -Wall -g -c -o $@ $<

ast.o: ast.c ast.h utils/string.h
	gcc -Wall -g -c -o $@ $<