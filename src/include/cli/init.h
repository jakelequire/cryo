#ifndef INIT_H
#define INIT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(name, mode) _mkdir(name)
#else
#include <unistd.h>
#endif




// Main functions
void init_command(void);
void create_cryo_config(void);
void create_cryo_main(void);
void create_cryo_init(void);

// Helper functions
void create_directory(const char* name);
void create_file(char* filename, char* data);
void open_root_dir(void);

#endif // INIT_H