#ifndef RUNTIME_H
#define RUNTIME_H
/*------ <includes> ------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <time.h>
/*---<custom_includes>---*/
/*---------<end>---------*/
#ifdef __linux__
#include <pthread.h>
#include <sys/inotify.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <process.h>  // For _beginthreadex and _endthreadex
#include <tlhelp32.h>
#endif

#ifdef __linux__
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#endif

void cryo_runtime(void);
void terminate_running_instance(const char *process_name);
bool should_ignore_event(const char *event_name);
#ifdef __linux__
void* read_user_input(void* arg);
#elif defined(_WIN32) || defined(_WIN64)
unsigned __stdcall read_user_input(void* arg);
#endif

#endif // RUNTIME_H
