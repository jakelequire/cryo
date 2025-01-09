/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use this file except in compliance with the License.          *
 *    You may obtain a copy of the License at                                   *
 *                                                                              *
 *    http://www.apache.org/licenses/LICENSE-2.0                                *
 *                                                                              *
 *    Unless required by applicable law or agreed to in writing, software       *
 *    distributed under the License is distributed on an "AS IS" BASIS,         *
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 *    See the License for the specific language governing permissions and       *
 *    limitations under the License.                                            *
 *                                                                              *
 ********************************************************************************/
#include "tools/utils/buildStats.h"
#include "tools/logger/logger_config.h"

// Helper macro for padding calculation
#define PRINT_PADDED(label, value, format)                                              \
    do                                                                                  \
    {                                                                                   \
        char temp[256];                                                                 \
        snprintf(temp, sizeof(temp), format, value);                                    \
        int padding = CONTENT_WIDTH - strlen(label) - strlen(temp);                     \
        printf(GREEN "║ " YELLOW "%s" COLOR_RESET "%s%*s" GREEN "     ║\n" COLOR_RESET, \
               label, temp, padding, "");                                               \
    } while (0)

// Helper macro without using colors
#define PRINT_PADDED_UNFORMATTED(label, value, format)              \
    do                                                              \
    {                                                               \
        char temp[256];                                             \
        snprintf(temp, sizeof(temp), format, value);                \
        int padding = CONTENT_WIDTH - strlen(label) - strlen(temp); \
        if (padding < 0)                                            \
            padding = 0;                                            \
        printf("║ %s%s%*s     ║\n", label, temp, padding, "");      \
    } while (0)

BuildStats *createBuildStats()
{
    BuildStats *stats = (BuildStats *)malloc(sizeof(BuildStats));
    if (!stats)
    {
        fprintf(stderr, "Error: Failed to allocate memory for BuildStats\n");
        return NULL;
    }
    return stats;
}

void *addCompilerSettings(BuildStats *stats, CompilerSettings *settings)
{
    // We can take some of the info from the CompilerSettings struct and add it to the BuildStats struct
    const char *buildType = BuildTypeToString(settings->buildType);
    strcpy(stats->build_type, buildType);

    int compiledFiles = settings->totalFiles;
    stats->files_compiled = compiledFiles;

    const char *compilerVersion = settings->version;
    strcpy(stats->compiler_version, compilerVersion);

    return stats;
}

void addElapsedTime(BuildStats *stats, double elapsed)
{
    stats->elapsed = elapsed;
}

void *getSystemInfo(BuildStats *stats)
{
    // Get peak memory usage
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    stats->peak_memory = usage.ru_maxrss * 1024; // Convert to bytes

    // Get current timestamp
    time_t now = time(NULL);
    strftime(stats->timestamp, sizeof(stats->timestamp),
             "%Y-%m-%d %H:%M:%S", localtime(&now));

    // Get number of CPU cores
    stats->cpu_cores_used = sysconf(_SC_NPROCESSORS_ONLN);

    // Alternative method to get CPU usage using getrusage
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) == 0)
    {
        // Calculate CPU usage based on user and system time
        double user_time = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0;
        double sys_time = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1000000.0;
        double total_time = user_time + sys_time;

        // Calculate percentage based on elapsed time
        if (stats->elapsed > 0)
        {
            stats->cpu_usage = (total_time / stats->elapsed / stats->cpu_cores_used) * 100.0;

            // Cap at 100% per core
            if (stats->cpu_usage > 100.0)
            {
                stats->cpu_usage = 100.0;
            }
        }
        else
        {
            stats->cpu_usage = 0.0;
        }
    }
    else
    {
        // Fallback if getrusage fails
        stats->cpu_usage = -1.0;
        fprintf(stderr, "Warning: Could not determine CPU usage\n");
    }

    return stats;
}

void formatSize(long bytes, char *buffer)
{
    const char *units[] = {"B", "KB", "MB", "GB"};
    int unit = 0;
    double size = bytes;

    while (size >= 1024 && unit < 3)
    {
        size /= 1024;
        unit++;
    }

    sprintf(buffer, "%.2f %s", size, units[unit]);
}

void printBuildStats(BuildStats *stats)
{
    DEBUG_PRINT_FILTER({
        char memory_str[32];
        formatSize(stats->peak_memory, memory_str);

        // Convert seconds to minutes and seconds if elapsed time > 60 seconds
        int minutes = (int)stats->elapsed / 60;
        float seconds = stats->elapsed - (minutes * 60);

        // Define box width and calculate padding
        const int BOX_WIDTH = 45; // Reduced width for cleaner look
        const int CONTENT_WIDTH = BOX_WIDTH - 2;

        printf("\n");
        // Top border with title
        printf(GREEN "╔═════════════════ BUILD SUMMARY ═════════════════╗\n" COLOR_RESET);

        // Timestamp
        PRINT_PADDED("Build completed at: ", stats->timestamp, "%s");

        // Build type
        PRINT_PADDED("Build type: ", stats->build_type, "%s");

        // Compiler version
        PRINT_PADDED("Compiler Version: ", stats->compiler_version, "%s");

        // Metrics section
        printf(GREEN "╠════════════════════ METRICS ════════════════════╣\n" COLOR_RESET);

        // Build time
        char time_str[64];
        snprintf(time_str, sizeof(time_str), "%.3f seconds", seconds);
        PRINT_PADDED("Build time: ", time_str, "%s");

        // Memory usage
        PRINT_PADDED("Peak memory: ", memory_str, "%s");

        // CPU usage
        char cpu_str[64];
        snprintf(cpu_str, sizeof(cpu_str), "%d (%.1f%% utilization)",
                 stats->cpu_cores_used, stats->cpu_usage);
        PRINT_PADDED("CPU cores: ", cpu_str, "%s");

        // Files compiled
        char files_str[32];
        snprintf(files_str, sizeof(files_str), "%ld", stats->files_compiled);
        PRINT_PADDED("Files compiled: ", files_str, "%s");

        printf(GREEN "╚═════════════════════════════════════════════════╝\n" COLOR_RESET);
        printf("\n");
        return;
    });
    {
        // Same as above but without colors if debug is disabled
        printBuiltStatsUnformatted(stats);
        return;
    }
}
#undef PRINT_PADDED

// Same as above but without colors
void printBuiltStatsUnformatted(BuildStats *stats)
{
    char memory_str[32];
    formatSize(stats->peak_memory, memory_str);

    // Convert seconds to minutes and seconds if elapsed time > 60 seconds
    int minutes = (int)stats->elapsed / 60;
    float seconds = stats->elapsed - (minutes * 60);

    // Define box width and calculate padding
    const int BOX_WIDTH = 45; // Reduced width for cleaner look
    const int CONTENT_WIDTH = BOX_WIDTH - 2;

    printf("\n");
    // Top border with title
    printf("╔═════════════════ BUILD SUMMARY ═════════════════╗\n");

    // Timestamp
    PRINT_PADDED_UNFORMATTED("Build completed at: ", stats->timestamp, "%s");

    // Build type
    PRINT_PADDED_UNFORMATTED("Build type: ", stats->build_type, "%s");

    // Compiler version
    PRINT_PADDED_UNFORMATTED("Compiler Version: ", stats->compiler_version, "%s");

    // Metrics section
    printf("╠════════════════════ METRICS ════════════════════╣\n");

    // Build time
    char time_str[64];
    snprintf(time_str, sizeof(time_str), "%.3f seconds", seconds);

    PRINT_PADDED_UNFORMATTED("Build time: ", time_str, "%s");

    // Memory usage
    PRINT_PADDED_UNFORMATTED("Peak memory: ", memory_str, "%s");

    // CPU usage
    char cpu_str[64];
    snprintf(cpu_str, sizeof(cpu_str), "%d (%.1f%% utilization)",
             stats->cpu_cores_used, stats->cpu_usage);

    PRINT_PADDED_UNFORMATTED("CPU cores: ", cpu_str, "%s");

    // Files compiled
    char files_str[32];
    snprintf(files_str, sizeof(files_str), "%ld", stats->files_compiled);

    PRINT_PADDED_UNFORMATTED("Files compiled: ", files_str, "%s");

    printf("╚═════════════════════════════════════════════════╝\n");
    printf("\n");
}
#undef PRINT_PADDED_UNFORMATTED
