CFLAGS = -ggdb -Wall -Wno-unused-but-set-variable
CC = gcc

shell: src/dict.c src/dict.h src/lexer.c src/lexer.h src/shell.c src/parser.c src/parser.h src/executables.h src/executables.c src/errors.h src/errors.c
	$(CC) $(CFLAGS)  -o $@ src/dict.c src/lexer.c src/shell.c src/parser.c src/executables.c src/errors.c

.PHONY: clean #Don't generate the file just do stuff

clean:
	rm -f *.o shell
