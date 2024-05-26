#ifndef BUILD_H
#define BUILD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <commands.h>


BuildArgs get_build_args(const char* build_args);

void build_command(int argc, char* argv[]);
void single_build(char* filename);
char* readFile(const char* path);


#endif

