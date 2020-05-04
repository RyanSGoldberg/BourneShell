#include "errors.h"
#include "stdio.h"
#include <stdlib.h>

void warning(char *message){
	fprintf(stderr, "WARNING: %s\n",message);
}
void severe(char *message){
	fprintf(stderr, "SEVERE: %s\n",message);
	exit(1);
}
