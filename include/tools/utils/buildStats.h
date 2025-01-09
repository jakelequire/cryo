
#ifndef BUILD_STATS_H
#define BUILD_STATS_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#include "tools/macros/consoleColors.h"
#include "settings/compilerSettings.h"

typedef struct
{
    double elapsed;            // Total elapsed time
    long peak_memory;          // Peak memory usage in bytes
    char build_type[32];       // Debug/Release/etc
    int cpu_cores_used;        // Number of CPU cores used
    double cpu_usage;          // CPU usage percentage
    char timestamp[64];        // Build timestamp
    long files_compiled;       // Number of files compiled
    char compiler_version[32]; // Compiler version used
} BuildStats;

void printBuildStats(BuildStats *stats);
void printBuiltStatsUnformatted(BuildStats *stats);

BuildStats *createBuildStats();
void *getSystemInfo(BuildStats *stats);
void *addCompilerSettings(BuildStats *stats, CompilerSettings *settings);
void addElapsedTime(BuildStats *stats, double elapsed);

void formatSize(long bytes, char *buffer);

#endif // BUILD_STATS_H
