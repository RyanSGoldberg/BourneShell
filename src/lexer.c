#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "lexer.h"
#include "executables.h"

#define MAX_TOKENS 64
int lexer_debug_mode = 0;

char *str_slice(char *str, int start, int end);
void token_init(Dict *EXE, Token *t, char *value);
int isword(char c);

Lexer *lexer_new(Dict *EXE, char *to_tokenize) {
	Lexer *lexer = malloc(sizeof(Lexer));
	memset(lexer, 0, sizeof(Lexer));

	lexer->curr_token = 0;
	lexer->EXE = EXE;
	lexer->tokens = malloc(sizeof(Token) * MAX_TOKENS);
	memset(lexer->tokens, 0, sizeof(Token) * MAX_TOKENS);
	lexer_to_tokens(lexer, to_tokenize);
	return lexer;
};
void lexer_to_tokens(Lexer *lexer, char *source) {
	enum state {
		EOS, start, word, notword, quote, space, token_found
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
				} else if (!isword(source[s])) {
					state = notword;
				} else {
					state = word;
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
			case word:
				if (0 != isword(source[e])) {
					e++;
				} else {
					state = token_found;
				}
				break;
			case notword:
				if (!isword(source[e]) && '\"' != source[e] && !isspace(source[e]) && '\0' != source[e]) {
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
	for(int i = 0; i < MAX_TOKENS; i++){
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

// Returns true iff c is a 'word' type character [a-zA-Z0-9 \- / \. _]
int isword(char c){
	return isalnum(c) || '-' == c || '/' == c || '_' == c || '.' == c || '~' == c;
}

void token_init(Dict *EXE,  Token *t, char *value) {
	t->value = strdup(value);

	if (0 == strcmp(value, "\n")) {
		t->type = NEWLINE;
	} else if (0 == strcmp(value, "&&")) {
		t->type = AND_IF;
	} else if (0 == strcmp(value, "||")) {
		t->type = OR_IF;
	} else if (0 == strcmp(value, ";")) {
		t->type = SEMI;
	} else {
		t->type = STRING;
	}
}