#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dict.h"
#include "lexer.h"
#include "parser.h"
#include "executables.h"

void shell_loop();
char* shell_read_line();
Dict *shell_load_executables(char* PATH);

void shell_loop(){
    Dict *ENV = dict_new();
    dict_add(ENV, "USER", getenv("USER"));
    dict_add(ENV, "PWD", getenv("PWD"));
	dict_add(ENV, "PATH", getenv("PATH"));
	dict_add(ENV, "HOME", getenv("HOME"));
	dict_add(ENV, "?", "0");

	Dict *EXE = load_executables(dict_get(ENV, "PATH"));

    char *line;
    int status = 0;

    do{
        if(!lexer_debug_mode && !parser_debug_mode){
            printf("%s@shell:%s$ ", dict_get(ENV, "USER"),dict_get(ENV, "PWD"));
        }else{
        	printf("\n");
        }

        // Reads in the line
        line  = shell_read_line();

        // Breaks the line into tokens
        Lexer* lexer = lexer_new(EXE, line);

        // Parses and executes the line
        Parser *parser = parser_new(lexer);
		parser_parse(parser, ENV);
		status = parser_execute(ENV, parser);

		// Saves the exit status code
		char temp[4];
		sprintf(temp, "%d", status);
		dict_add(ENV, "?", temp);

        // Frees the memory
        free(line);
        lexer_free(lexer);
        parser_free(parser);
    }while (1);
	dict_free(ENV);
	dict_free(EXE);
}

char* shell_read_line(){
    char *line_buffer = malloc(sizeof(char*)*256);
    memset(line_buffer, 0,sizeof(char**)*256 );
    fgets(line_buffer,256, stdin);
    *strchr(line_buffer, '\n') = '\0';
    return line_buffer;
}

int main(int argc, char** argv){
    for(int i = 1; i < argc; i++){
        if(0 == strcmp(argv[i], "--debug-lexer")){
            lexer_debug_mode = 1;
        }
		if(0 == strcmp(argv[i], "--debug-parser")){
			parser_debug_mode = 1;
		}
		if(0 == strcmp(argv[i], "--debug")){
			lexer_debug_mode = 1;
			parser_debug_mode = 1;
		}
    }
    shell_loop();
    return 0;
}
