#ifndef COMPILER_H
#define COMPILER_H
/*------ <includes> ------*/
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#define popen _popen
#define pclose _pclose
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
/*------ </includes> -----*/
void run_compiler(const char *input_file);

#endif