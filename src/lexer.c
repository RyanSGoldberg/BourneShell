#include "lexer.h"
#include "stdlib.h"
#include "string.h"
#include <ctype.h>
#include <stdio.h>
#include "executables.h"

#define NUM_TOKENS 64
int lexer_debug_mode = 0;

char *str_slice(char *str, int start, int end);
void token_init(Dict *EXE, Token *t, char *value);

Lexer *lexer_new(Dict *EXE, char *to_tokenize) {
	Lexer *lexer = malloc(sizeof(Lexer));
	memset(lexer, 0, sizeof(Lexer));

	lexer->curr_token = 0;
	lexer->EXE = EXE;
	lexer->tokens = malloc(sizeof(Token) * NUM_TOKENS);
	memset(lexer->tokens, 0, sizeof(Token) * NUM_TOKENS);
	lexer_to_tokens(lexer, to_tokenize);
	return lexer;
};
void lexer_to_tokens(Lexer *lexer, char *source) {
	enum state {
		EOS, start, alnum, nalnum, quote, space, token_found
	};

	// The current state the lexer is in
	enum state state = start;
	// The current token start, inclusive
	int s = 0;
	//The current token end, exclusive
	int e = 0;
	int n = 0;

	while (1) {
		switch (state) {
			case start:
				if ('\0' == source[s]) {
					state = EOS;
				} else if (isspace(source[s])) {
					state = space;
				} else if ('\"' == source[s]) {
					// Move past the opening quote
					s++;
					e++;
					state = quote;
				} else if (!isalnum(source[s]) && '-' != source[s] && '/' != source[s]  && '_' != source[s] && '.' != source[s]) {
					state = nalnum;
				} else {
					state = alnum;
				}
				break;
			case space:
				if (isspace(source[s])) {
					s++;
					e++;
				} else {
					state = start;
				}
				break;
			case quote:
				if ('\"' != source[e]) {
					e++;
				} else {
					state = token_found;
				}
				break;
			case alnum:
				if (isalnum(source[e]) || '-' == source[e] || '/' == source[e] || '_' == source[e] || '.' == source[e]) {
					e++;
				} else {
					state = token_found;
				}
				break;
			case nalnum:
				if (!isalnum(source[e]) && '\"' != source[e] && !isspace(source[e]) && '\0' != source[e]) {
					e++;
				} else {
					state = token_found;
				}
				break;
			case token_found:{
				// Makes a new token
				char *temp = str_slice(source, s, e);
				token_init(lexer->EXE, &lexer->tokens[n++], temp);
				free(temp);

				if (0 != lexer_debug_mode) {
					printf("%s:%d\n", lexer->tokens[n - 1].value, lexer->tokens[n - 1].type);
				}

				// Moves start and end up to the next token
				s = e;
				e = s;

				// FIXME: I don't love this control flow
				if ('\"' == source[e]) {
					s++;
					e++;
				}
				state = start;
			}
				break;
			case EOS:
				token_init(lexer->EXE, &lexer->tokens[n++], "\n");
				return;
		}
	};
}
Token *lexer_get_token(Lexer *lexer) {
	if (ZERO == lexer->tokens[lexer->curr_token].type) {
		return NULL;
	}
	return &lexer->tokens[lexer->curr_token++];
}
void lexer_free(Lexer *lexer) {
	for(int i = 0; i < NUM_TOKENS; i++){
		if(NULL != lexer->tokens[i].value){
			free(lexer->tokens[i].value);
		}
	}

	free(lexer->tokens);
	free(lexer);
}

// String slice, inclusive-exclusive
char *str_slice(char *str, int start, int end) {
	int n = end - start;
	char *buffer = malloc(sizeof(char) * 256);
	memcpy(buffer, &str[start], n);
	buffer[n] = '\0';
	return buffer;
}

void token_init(Dict *EXE,  Token *t, char *value) {
	t->value = strdup(value);

	//Determines the Token's type
	if(0 != is_executable(EXE, t->value)){
		t->type = COMMAND;
		return;
	}

	if (0 == strcmp(value, "\n")) {
		t->type = NEWLINE;
	} else if (0 == strcmp(value, "=")) {
		t->type = EQUAL;
	} else if (0 == strcmp(value, "&&")) {
		t->type = AND_IF;
	} else if (0 == strcmp(value, "||")) {
		t->type = OR_IF;
	} else if (0 == strcmp(value, ";")) {
		t->type = SEMI;
	} else {
		t->type = WORD;
	}
}

//FIXME: UNDERSCORE TOO