/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "../include/commands.h"

#define INITIAL_BUFFER_SIZE 1024
#define BUFFER_GROWTH_FACTOR 2

static CryoEnvironment *global_env = NULL; // Global pointer for signal handler

// Signal handler
static void handle_signal(int sig)
{
    fprintf(stderr, "[Cryo] Debug: Signal %d received\n", sig);
    if (global_env)
    {
        fprintf(stderr, "[Cryo] Debug: Setting running flag to 0\n");
        global_env->running = 0;
    }
    else
    {
        fprintf(stderr, "[Cryo] Debug: global_env is NULL in signal handler\n");
    }
}

// Function to setup signals
static int setup_signals(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;

    if (sigaction(SIGINT, &sa, NULL) == -1)
        return -1;
    if (sigaction(SIGTERM, &sa, NULL) == -1)
        return -1;
    if (sigaction(SIGQUIT, &sa, NULL) == -1)
        return -1;

    return 0;
}

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
    env->running = 1; // Initialize running flag
    env->lastWasEscape = 0;
    global_env = env; // Set global pointer for signal handler

    return env;
}

// Function to clean up the environment
static void cleanup_environment(CryoEnvironment *env)
{
    fprintf(stderr, "[Cryo] Debug: Starting cleanup\n");
    if (env)
    {
        // Restore terminal state first
        disable_raw_mode();
        fprintf(stderr, "[Cryo] Debug: Raw mode disabled\n");

        // Clear screen before switching back
        printf("\x1b[2J"); // Clear entire screen
        printf("\x1b[H");  // Move cursor to home position
        fflush(stdout);

        // Restore main screen buffer
        printf("\x1b[?1049l");
        fflush(stdout);
        fprintf(stderr, "[Cryo] Debug: Screen buffer restored\n");

        free(env->codeBuffer);
        free(env->completedBuffer);
        global_env = NULL;
        free(env);
        fprintf(stderr, "[Cryo] Debug: Memory freed\n");
    }
    else
    {
        fprintf(stderr, "[Cryo] Debug: Cleanup called with NULL env\n");
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

// Function to handle escape sequences
static int handle_escape_sequence(CryoEnvironment *env)
{
    char seq[32];
    ssize_t n;

    // Read the first character after ESC
    if ((n = read(STDIN_FILENO, &seq[0], 1)) != 1)
        return 0;

    // Read the second character
    if ((n = read(STDIN_FILENO, &seq[1], 1)) != 1)
        return 0;

    fprintf(stderr, "[Cryo] Debug: Escape sequence chars: [0]=%d ('%c'), [1]=%d ('%c')\n",
            seq[0], seq[0], seq[1], seq[1]);

    // Handle Shift+Enter sequence
    if (seq[0] == '[')
    {
        // Read until we find a terminal character
        char c;
        char sequence[16] = {0};
        int i = 0;

        while (i < 15 && read(STDIN_FILENO, &c, 1) == 1)
        {
            sequence[i++] = c;
            fprintf(stderr, "[Cryo] Debug: Additional char: %d ('%c')\n", c, c);

            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '~')
                break;
        }

        fprintf(stderr, "[Cryo] Debug: Full sequence: ESC[%s\n", sequence);

        // Check for any known Shift+Enter sequence variant
        if ((seq[1] == 'Z') ||                          // Simple Shift+Tab sequence
            (strstr(sequence, "13;2") != NULL) ||       // Modern terminal sequence
            (sequence[0] == '1' && sequence[1] == '3')) // Alternative sequence
        {
            if (ensure_buffer_capacity(env, env->bufferPos + 2) == -1)
                return -1;

            // Set escape flag to prevent normal enter processing
            env->lastWasEscape = 1;

            // Add newline to buffer
            env->codeBuffer[env->bufferPos++] = '\n';
            env->codeBuffer[env->bufferPos] = '\0';

            // Update display
            printf("\r\n> ");
            fflush(stdout);

            return 1;
        }
    }

    return 0;
}

// Add this function before exe_env_cmd
static int check_quit_command(const char *buffer)
{
    // Check if buffer is exactly "!q"
    return (buffer[0] == '!' && buffer[1] == 'q' && buffer[2] == '\0');
}

// Entry point to the `cryo env` command
int exe_env_cmd(void)
{
    fprintf(stderr, "[Cryo] Debug: Starting initialization...\n");

    // Initialize environment first
    CryoEnvironment *env = init_environment();
    if (!env)
    {
        fprintf(stderr, "Failed to initialize environment\n");
        return 1;
    }
    fprintf(stderr, "[Cryo] Debug: Environment initialized\n");

    // Setup signal handling
    if (setup_signals() == -1)
    {
        fprintf(stderr, "Failed to setup signal handlers\n");
        cleanup_environment(env);
        return 1;
    }
    fprintf(stderr, "[Cryo] Debug: Signals setup complete\n");

    // Setup terminal after signals are handled
    if (enable_raw_mode() == -1)
    {
        fprintf(stderr, "Failed to enable raw mode\n");
        cleanup_environment(env);
        return 1;
    }
    fprintf(stderr, "[Cryo] Debug: Raw mode enabled\n");

    // Clear the screen and set up the terminal
    printf("\x1b[2J");     // Clear entire screen
    printf("\x1b[H");      // Move cursor to home position
    printf("\x1b[?1049h"); // Enter alternate screen buffer
    fflush(stdout);

    // fprintf(stderr, "[Cryo] Debug: Screen buffer switched\n"); (Debugging line)

    // Print title on the first line
    printf("Cryo Environment (Shift+Enter for new line, Enter to execute)\r\n");
    printf("Type !q to quit\r\n");
    fflush(stdout);

    // Start prompt on a fresh line
    printf("> ");
    fflush(stdout);

    while (env->running)
    {
        char c;
        ssize_t nread;

        // Use select to check for input availability
        fd_set rfds;
        struct timeval tv;

        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);

        tv.tv_sec = 1; // 1 second timeout
        tv.tv_usec = 0;

        int retval = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);

        if (retval == -1)
        {
            if (errno == EINTR)
            {
                // Interrupted by signal, check running flag
                fprintf(stderr, "[Cryo] Debug: Select interrupted by signal\n");
                continue;
            }
            fprintf(stderr, "[Cryo] Debug: Select error: %s\n", strerror(errno));
            break;
        }

        if (retval == 0)
        {
            // Timeout, check running flag and continue
            continue;
        }

        nread = read(STDIN_FILENO, &c, 1);
        if (nread == -1)
        {
            if (errno == EINTR)
            {
                // Interrupted by signal, check running flag
                continue;
            }
            fprintf(stderr, "[Cryo] Debug: Read error: %s\n", strerror(errno));
            break;
        }

        if (nread == 0)
        {
            // EOF
            continue;
        }

        // fprintf(stderr, "[Cryo] Debug: Read character: %d\n", c); (Debugging line)

        if (c == 27) // ESC character
        {
            int result = handle_escape_sequence(env);
            if (result < 0)
            {
                fprintf(stderr, "[Cryo] Debug: Escape sequence error\n");
                break;
            }
            if (result > 0)
            {
                continue;
            }
        }

        // Handle Enter key
        if (c == '\r' || c == '\n')
        {
            // Skip if this is part of an escape sequence
            if (env->lastWasEscape)
            {
                env->lastWasEscape = 0;
                continue;
            }

            if (env->bufferPos > 0)
            {
                // Check for quit command first
                if (check_quit_command(env->codeBuffer))
                {
                    fprintf(stderr, "[Cryo] Debug: Quit command received\n");
                    env->running = 0;
                    printf("\r\nExiting...\r\n");
                    fflush(stdout);
                    break;
                }

                // Complete the buffer
                env->completedBuffer = strdup(env->codeBuffer);
                if (!env->completedBuffer)
                {
                    fprintf(stderr, "[Cryo] Debug: Failed to allocate completedBuffer\n");
                    break;
                }

                printf("\nBuffer completed. Content:\r\n%s\r\n", env->completedBuffer);
                env->bufferPos = 0;
                env->codeBuffer[0] = '\0';
                printf("\r\n> ");
                fflush(stdout);
            }
            else
            {
                // Empty line, just print a new prompt
                printf("\r\n> ");
                fflush(stdout);
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
                fflush(stdout);
            }
            continue;
        }

        // Add character to buffer
        if (ensure_buffer_capacity(env, env->bufferPos + 2) == -1)
        {
            fprintf(stderr, "[Cryo] Debug: Buffer capacity error\n");
            break;
        }
        env->codeBuffer[env->bufferPos++] = c;
        env->codeBuffer[env->bufferPos] = '\0';
        printf("%c", c);
        fflush(stdout);
    }

    fprintf(stderr, "[Cryo] Debug: Main loop ended, cleaning up...\n");
    cleanup_environment(env);
    fprintf(stderr, "[Cryo] Debug: Cleanup complete\n");
    return 0;
}
