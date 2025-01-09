#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#define ORIGINAL_PORT 4389
#define PROXY_PORT 4390
#define BUFFER_SIZE 8192
#define MAX_CLIENTS 10

// ANSI color codes
#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_BRIGHT "\x1b[1m"

// Global flag for cleanup
volatile sig_atomic_t running = 1;

void handle_signal(int sig)
{
    running = 0;
}

// Utility function to get current timestamp
void get_timestamp(char *buffer, size_t size)
{
    time_t now;
    struct tm *tm_info;

    time(&now);
    tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Try to pretty print if the data looks like JSON
void print_data(const char *direction, const char *data, ssize_t length)
{
    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));

    // Null terminate the data for string operations
    char *data_copy = (char *)malloc(length + 1);
    memcpy(data_copy, data, length);
    data_copy[length] = '\0';

    // Print the header with direction
    if (strcmp(direction, "LSP -> Compiler") == 0)
    {
        printf("\n%s%s[%s] %s%s:\n", COLOR_BRIGHT, COLOR_YELLOW, timestamp, direction, COLOR_RESET);
    }
    else
    {
        printf("\n%s%s[%s] %s%s:\n", COLOR_BRIGHT, COLOR_GREEN, timestamp, direction, COLOR_RESET);
    }

    // Check if it might be JSON (starts with { or [)
    if (data_copy[0] == '{' || data_copy[0] == '[')
    {
        // Simple JSON pretty print (basic indentation)
        int indent = 0;
        for (size_t i = 0; i < length; i++)
        {
            char c = data_copy[i];

            if (c == '{' || c == '[')
            {
                printf("%c\n", c);
                indent += 2;
                for (int j = 0; j < indent; j++)
                    printf(" ");
            }
            else if (c == '}' || c == ']')
            {
                printf("\n");
                indent -= 2;
                for (int j = 0; j < indent; j++)
                    printf(" ");
                printf("%c", c);
            }
            else if (c == ',')
            {
                printf("%c\n", c);
                for (int j = 0; j < indent; j++)
                    printf(" ");
            }
            else
            {
                printf("%c", c);
            }
        }
        printf("\n");
    }
    else
    {
        // Print raw data if not JSON
        printf("%.*s\n", (int)length, data_copy);
    }

    free(data_copy);
}

int main()
{
    int proxy_socket, client_socket, server_socket;
    struct sockaddr_in proxy_addr, client_addr, server_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE];
    int opt = 1;

    // Setup signal handling for clean exit
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Create proxy socket
    if ((proxy_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Proxy socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(proxy_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Setup proxy address
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = INADDR_ANY;
    proxy_addr.sin_port = htons(PROXY_PORT);

    // Bind proxy socket
    if (bind(proxy_socket, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) < 0)
    {
        perror("Proxy bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(proxy_socket, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("%s%sPort monitor running:%s\n", COLOR_BRIGHT, COLOR_BLUE, COLOR_RESET);
    printf("Listening on port %d and forwarding to port %d\n", PROXY_PORT, ORIGINAL_PORT);
    printf("Press Ctrl+C to exit\n\n");

    while (running)
    {
        // Accept client connection
        if ((client_socket = accept(proxy_socket, (struct sockaddr *)&client_addr, &addr_len)) < 0)
        {
            if (errno == EINTR)
                continue; // Interrupted by signal
            perror("Accept failed");
            break;
        }

        printf("%s%sNew client connection established%s\n", COLOR_BRIGHT, COLOR_MAGENTA, COLOR_RESET);

        // Create connection to original server
        if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Server socket creation failed");
            close(client_socket);
            continue;
        }

        // Setup server address
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(ORIGINAL_PORT);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        // Connect to original server
        if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            perror("Connection to original server failed");
            close(client_socket);
            close(server_socket);
            continue;
        }

        pid_t pid = fork();
        if (pid == 0)
        {                        // Child process
            close(proxy_socket); // Close listening socket in child

            fd_set readfds;
            while (running)
            {
                FD_ZERO(&readfds);
                FD_SET(client_socket, &readfds);
                FD_SET(server_socket, &readfds);

                int max_fd = (client_socket > server_socket ? client_socket : server_socket) + 1;

                if (select(max_fd, &readfds, NULL, NULL, NULL) < 0)
                {
                    if (errno == EINTR)
                        continue; // Interrupted by signal
                    perror("Select failed");
                    break;
                }

                // Handle client -> server
                if (FD_ISSET(client_socket, &readfds))
                {
                    ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
                    if (bytes_read <= 0)
                        break;

                    print_data("LSP -> Compiler", buffer, bytes_read);
                    send(server_socket, buffer, bytes_read, 0);
                }

                // Handle server -> client
                if (FD_ISSET(server_socket, &readfds))
                {
                    ssize_t bytes_read = recv(server_socket, buffer, BUFFER_SIZE, 0);
                    if (bytes_read <= 0)
                        break;

                    print_data("Compiler -> LSP", buffer, bytes_read);
                    send(client_socket, buffer, bytes_read, 0);
                }
            }

            close(client_socket);
            close(server_socket);
            exit(EXIT_SUCCESS);
        }

        // Parent process
        close(client_socket);
        close(server_socket);
    }

    close(proxy_socket);
    printf("\n%sMonitor shutting down...%s\n", COLOR_BRIGHT, COLOR_RESET);
    return 0;
}
