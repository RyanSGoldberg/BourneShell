#ifndef SHELL_V1_DICT_H
#define SHELL_V1_DICT_H

typedef struct Dict Dict;

Dict *dict_new();
void  dict_free(Dict *d);
void  dict_add(Dict *d, char *key, char *value);
int   dict_remove(Dict *d, char *key);
int   dict_contains(Dict *d, char *key);
char *dict_get(Dict *d, char *key);
void  dict_print_all(Dict *d);

#endif