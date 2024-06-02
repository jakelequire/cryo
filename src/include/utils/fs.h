#ifndef FS_H
#define FS_H
/*------ <includes> ------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
/*---------<end>---------*/
char* readFile(const char* path);
#define CUR_ROOT_DIR_ABS
#define CUR_ROOT_DIR_REL
#endif
