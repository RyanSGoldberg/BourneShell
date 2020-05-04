#ifndef LEXER_SEEN
#define LEXER_SEEN

#include "dict.h"


enum token_type {ZERO, STRING, AND_IF, OR_IF, SEMI, NEWLINE};

typedef struct Token{
    enum token_type type;
    char* value;
} Token;

typedef struct Lexer{
    int curr_token;
    Token* tokens;
    Dict *EXE;
}Lexer;

extern int lexer_debug_mode;
Lexer *lexer_new(Dict *EXE, char *to_tokenize);
void lexer_to_tokens(Lexer *lexer, char *source);
Token *lexer_get_token(Lexer *lexer);
void lexer_free(Lexer *lexer);

#endif
