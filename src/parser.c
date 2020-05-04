#include "parser.h"
#include <stdlib.h>
#include "string.h"
#include "executables.h"
#include "stdio.h"
#include "errors.h"
#include "dict.h"

#include <unistd.h> // Fork and exec
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_COMPOUND_LENGTH 64

SimpleCommand *simple_command_init(SimpleCommand *simpleCommand, char *command, enum command_type type);
void simple_command_free(SimpleCommand *command);

void parser_parse(Parser *parser, Dict *ENV){
	enum states {start, command_found, error_found};

	Token *token = NULL;
	enum command_type type = ALL;

	SimpleCommand* simpleCommand = NULL;

	enum states state = start;
	do{
		switch(state){
			case start:
				token = lexer_get_token(parser->lexer);

				if(COMMAND == token->type){
					simpleCommand = simple_command_init(parser->commands + parser->num_commands,token->value, type);
					state = command_found;
					token = lexer_get_token(parser->lexer);
				}else{
					// The first token is not a command
					char temp[256];
					sprintf(temp, "Command %s not found",token->value);
					warning(temp);
					state = error_found;
				}
				break;
			case command_found:
				if(AND_IF == token->type || OR_IF == token->type || SEMI == token->type || NEWLINE == token->type){
					// Saves the last command
					if(NULL != simpleCommand){
						parser->num_commands++;
					}

					switch(token->type){
						case AND_IF:
							type = IF_TRUE;
							break;
						case OR_IF:
							type = IF_FALSE;
							break;
						case SEMI:
							type = ALL;
							break;
						case NEWLINE:
							return;
							//TODO: Make sure there is a command in the last position i.e ls; won't break
						default:
							severe("Uh oh, something is seriously wrong here");
					}

					state = start;
				}else{
					if(0 == strcmp(token->value, "$")){
						char *key = lexer_get_token(parser->lexer)->value;
						if(dict_contains(ENV, key)){
							simpleCommand->argv[simpleCommand->argc] = strdup(dict_get(ENV, key));
							simpleCommand->argc++;
						}else{
							char temp[256];
							sprintf(temp, "Key %s not found",key);
							warning(temp);
							state = error_found;
						}
					}else{
						simpleCommand->argv[simpleCommand->argc] = strdup(token->value);
						simpleCommand->argc++;
					}

					token = lexer_get_token(parser->lexer);
				}
				break;
			case error_found:
				// Find the start of the next command
				if(AND_IF == token->type || OR_IF == token->type || SEMI == token->type || NEWLINE == token->type){
					// Clears the bad SimpleCommand
					simpleCommand = NULL;
					state = command_found;
				}else{
					token = lexer_get_token(parser->lexer);
				}
				break;
		}
	}while(NULL != token);
}
Parser *parser_new(Lexer *lexer){
	Parser *parser = malloc(sizeof(Parser));
	memset(parser, 0, sizeof(Parser));

	parser->lexer = lexer;
	parser->commands = malloc(sizeof(SimpleCommand)*MAX_COMPOUND_LENGTH);
	return parser;
}
void parser_free(Parser *parser){
//	Free the simple commands
	for(int i = 0; i < parser->num_commands; i++){
		simple_command_free(parser->commands + i);
	}

	free(parser->commands);
	free(parser);
}
int parser_execute(Dict *ENV, Parser *parser){

	int status = 0;
	for(int i = 0; i < parser->num_commands; i++){
//		printf("Command: %s\n", parser->commands[i].command);
//		printf("Type: %d\n", parser->commands[i].type);
//		printf("Argc: %d\nArgv: ", parser->commands[i].argc);
//		for(int j = 0; j < parser->commands[i].argc; j++){
//			printf("%s ", parser->commands[i].argv[j]);
//		}
//
//		printf("\n______________________________\n");
		if(IF_TRUE == parser->commands[i].type){
			if(status != 0)
				return 1;
		}else if(IF_FALSE == parser->commands[i].type){
			if(status == 0)
				return 1;
		}


		if(0 == strcmp("exit", parser->commands[i].command)){
			exit_shell(42);
		}else if(0 == strcmp("set", parser->commands[i].command)){
			set(ENV, parser->commands[i].argv[1], parser->commands[i].argv[2]);
		}else if(0 == strcmp("unset", parser->commands[i].command)){
			unset(ENV, parser->commands[i].argv[1]);
		}else if(0 == strcmp("env", parser->commands[i].command)){
			env(ENV);
		}else if(0 == strcmp("cd", parser->commands[i].command)){
			cd(ENV, parser->commands[i].argv[1]);
		}else{
			//FIXME TEMPORARY
			if(0 == strcmp("ls", parser->commands[i].command)){
				parser->commands[i].argv[parser->commands[i].argc++] = strdup("--color");
				parser->commands[i].argv[parser->commands[i].argc++] = strdup("-a");
			}

			pid_t pid = fork();
			if(0 == pid){
				// The child

				int s;
				if(NULL != strchr(parser->commands[i].command, '/')){
					s = execv(parser->commands[i].command,  parser->commands[i].argv);
				}else{
					s = execv(dict_get(parser->lexer->EXE, parser->commands[i].command),  parser->commands[i].argv);
				}

				if(-1 == s){
					severe("Something has gone wrong! Take a look at your exec");
				}
			}else{
				// The parent
				wait(&status);
			}
		}
	}
	return 0;
}

SimpleCommand *simple_command_init(SimpleCommand *simpleCommand, char *command, enum command_type type){
	simpleCommand->command = strdup(command);

	simpleCommand->argv = malloc(sizeof(char*)*32);
	memset(simpleCommand->argv, 0, sizeof(char*)*32);
	simpleCommand->argv[0] = strdup(command);

	simpleCommand->argc = 1;

	simpleCommand->type = type;
	return simpleCommand;
}
void simple_command_free(SimpleCommand *simpleCommand){
	free(simpleCommand->command);
	for(int i = 0; i < simpleCommand->argc; i++){
		free(simpleCommand->argv[i]);
	}
	free(simpleCommand->argv);
}