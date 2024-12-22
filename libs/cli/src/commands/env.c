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
#include "commands.h"

#define INITIAL_BUFFER_SIZE 1024
#define BUFFER_GROWTH_FACTOR 2

typedef struct CryoEnvironment
{
    char *codeBuffer;
    size_t bufferSize;
    size_t bufferPos;
    char *completedBuffer;
} CryoEnvironment;

// Function to initialize the environment
static CryoEnvironment *init_environment(void)
{
    CryoEnvironment *env = (CryoEnvironment *)malloc(sizeof(CryoEnvironment));
    if (!env)
        return NULL;

    env->codeBuffer = (char *)malloc(INITIAL_BUFFER_SIZE);
    if (!env->codeBuffer)
    {
        free(env);
        return NULL;
    }

    env->bufferSize = INITIAL_BUFFER_SIZE;
    env->bufferPos = 0;
    env->codeBuffer[0] = '\0';
    env->completedBuffer = NULL;

    return env;
}

// Function to clean up the environment
static void cleanup_environment(CryoEnvironment *env)
{
    if (env)
    {
        free(env->codeBuffer);
        free(env->completedBuffer);
        free(env);
    }
}

// Function to grow the buffer if needed
static int ensure_buffer_capacity(CryoEnvironment *env, size_t needed_size)
{
    if (needed_size >= env->bufferSize)
    {
        size_t new_size = env->bufferSize * BUFFER_GROWTH_FACTOR;
        while (new_size <= needed_size)
        {
            new_size *= BUFFER_GROWTH_FACTOR;
        }

        char *new_buffer = (char *)realloc(env->codeBuffer, new_size);
        if (!new_buffer)
            return -1;

        env->codeBuffer = new_buffer;
        env->bufferSize = new_size;
    }
    return 0;
}

// Function to setup raw terminal mode
static struct termios orig_termios;

static void disable_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static int enable_raw_mode(void)
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        return -1;
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        return -1;
    return 0;
}

// Entry point to the `cryo env` command
int exe_env_cmd(void)
{
    CryoEnvironment *env = init_environment();
    if (!env)
    {
        fprintf(stderr, "Failed to initialize environment\n");
        return 1;
    }

    if (enable_raw_mode() == -1)
    {
        fprintf(stderr, "Failed to enable raw mode\n");
        cleanup_environment(env);
        return 1;
    }

    printf("Cryo Environment (Shift+Enter for new line, Enter to execute)\n> ");

    char c;
    int escape_sequence = 0;
    char escape_buffer[3] = {0};

    while (read(STDIN_FILENO, &c, 1) == 1)
    {
        if (escape_sequence)
        {
            escape_buffer[escape_sequence - 1] = c;
            if (escape_sequence == 2)
            {
                // Handle Shift+Enter (typically sent as \x1b[13;2u or similar)
                if (strncmp(escape_buffer, "[Z", 2) == 0)
                {
                    // Add newline to buffer
                    if (ensure_buffer_capacity(env, env->bufferPos + 2) == -1)
                        break;
                    env->codeBuffer[env->bufferPos++] = '\n';
                    env->codeBuffer[env->bufferPos] = '\0';
                    printf("\n> ");
                }
                escape_sequence = 0;
            }
            else
            {
                escape_sequence++;
            }
            continue;
        }

        if (c == 27)
        { // ESC character
            escape_sequence = 1;
            continue;
        }

        if (c == '\r' || c == '\n')
        {
            if (env->bufferPos > 0)
            {
                // Store the completed buffer
                env->completedBuffer = strdup(env->codeBuffer);
                if (!env->completedBuffer)
                    break;

                printf("\nBuffer completed. Content:\n%s\n", env->completedBuffer);
                // Reset the code buffer
                env->bufferPos = 0;
                env->codeBuffer[0] = '\0';
                printf("\n> ");
            }
            continue;
        }

        // Handle backspace
        if (c == 127)
        {
            if (env->bufferPos > 0)
            {
                env->bufferPos--;
                env->codeBuffer[env->bufferPos] = '\0';
                printf("\b \b");
            }
            continue;
        }

        // Add character to buffer
        if (ensure_buffer_capacity(env, env->bufferPos + 2) == -1)
            break;
        env->codeBuffer[env->bufferPos++] = c;
        env->codeBuffer[env->bufferPos] = '\0';
        printf("%c", c);
    }

    cleanup_environment(env);
    return 0;
}
