#ifndef EXECUTABLES_SEEN
#define EXECUTABLES_SEEN

#include "dict.h"

const extern char *builtin_commands[];
const extern int num_builtin_commands;

int env(Dict *ENV);
int exit_shell(int status);
void set(Dict *ENV, char* var, char* val);
void unset(Dict *ENV, char* var);
void cd(Dict *ENV, char *new_path);
int is_executable(Dict *EXE, char *command);
char *get_executable_path(Dict *EXE, char *command);
Dict *load_executables(char* PATH);

#endif
