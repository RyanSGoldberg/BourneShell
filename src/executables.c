#include <sys/stat.h>
#include <sys/param.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "executables.h"
#include "errors.h"

const int num_builtin_commands = 5;
const char *builtin_commands[5] = {"exit", "set", "unset", "env", "cd"};

int env(Dict *ENV){
	dict_print_all(ENV);
	return 0;
}
int exit_shell(int status){
	exit(status);
}
void set(Dict *ENV, char* var, char* val){
	dict_add(ENV, var, val);
}
void unset(Dict *ENV, char* var){
	dict_remove(ENV, var);
}
void cd(Dict *ENV, char *new_path){
	char buf[256];
	int status = 0;

	if('/' == new_path[0]){
		// An absolute path
		status = chdir(new_path);
		if(-1 == status){
			warning("cd: No such file or directory");
		}

	}else{
		// A relative path
		char temp[512];
		sprintf(temp, "%s/%s", dict_get(ENV, "PWD"), new_path);
		status = chdir(temp);
		if(-1 == status){
			warning("cd: No such file or directory");
		}

	}

	set(ENV, "PWD", buf);
}
int is_executable(Dict *EXE, char *command){
	for (int i = 0; i < num_builtin_commands; i++) {
		if (0 == strcmp(command, builtin_commands[i])) {
			// A builtin command
			return 1;
		}
	}

	// An executable
	if(NULL != dict_get(EXE, command)){
		return 1;
	}

	struct stat sb;
	//Path specified executable
	if (stat(command, &sb) == 0 && sb.st_mode & S_IXUSR){
		return 1;
	}

	// Not an executable
	return 0;
}

char *get_executable_path(Dict *EXE, char *command){
	return dict_get(EXE, command);
}

Dict *load_executables(char* PATH){
	Dict *EXE = dict_new();
	struct dirent *dir;
	DIR *d;

	for(char *dir_path = strtok(PATH, ":"); NULL != dir_path;  dir_path = strtok(NULL, ":")){
		d = opendir(dir_path);

		if (NULL != d){
			for(dir = readdir(d); NULL != dir; dir = readdir(d)){
				if('.' != dir->d_name[0]){
					char temp[MAXPATHLEN];
					char *name = dir->d_name;
					sprintf(temp, "%s/%s", dir_path, name);
					dict_add(EXE, name, temp);
				}
			}
			closedir(d);
		}
	}

	return EXE;
}
