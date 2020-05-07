#ifndef PARSER_SEEN
#define PARSER_SEEN

#include "lexer.h"
#include "dict.h"

enum command_type {ALL, IF_TRUE, IF_FALSE};

typedef struct SimpleCommand{
	enum command_type type;
	char *command;
	int argc;
	char **argv;
	char **redirects;
}SimpleCommand;


typedef struct Parser{
	Lexer *lexer;
	SimpleCommand *commands;
	int num_commands;
}Parser;

extern int parser_debug_mode;
Parser *parser_new(Lexer* lexer);
void parser_parse(Parser *parser, Dict *ENV);
void parser_free(Parser *parser);
int parser_execute(Dict *ENV, Parser *parser);

#endif
