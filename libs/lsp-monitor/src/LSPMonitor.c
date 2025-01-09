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
#include <sys/types.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <arpa/inet.h>

#define ORIGINAL_PORT 9000
#define PROXY_PORT 9001
#define BUFFER_SIZE 8192
#define MAX_CLIENTS 10

// ANSI color codes
#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_BRIGHT "\x1b[1m"

// Global flag for cleanup
volatile sig_atomic_t running = 1;

// Structure to store connection information
typedef struct
{
    pid_t client_pid;
    char client_host[NI_MAXHOST];
    char client_service[NI_MAXSERV];
    time_t connect_time;
} connection_info;

void handle_signal(int sig)
{
    running = 0;
}

// Enhanced timestamp function with microseconds
void get_timestamp(char *buffer, size_t size)
{
    struct timespec ts;
    struct tm *tm_info;

    clock_gettime(CLOCK_REALTIME, &ts);
    tm_info = localtime(&ts.tv_sec);
    snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d.%06ld",
             tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
             ts.tv_nsec / 1000);
}

// Get process name by PID
void get_process_name(pid_t pid, char *name, size_t size)
{
    char proc_path[256];
    snprintf(proc_path, sizeof(proc_path), "/proc/%d/comm", pid);

    FILE *f = fopen(proc_path, "r");
    if (f)
    {
        if (fgets(name, size, f) != NULL)
        {
            // Remove newline if present
            size_t len = strlen(name);
            if (len > 0 && name[len - 1] == '\n')
            {
                name[len - 1] = '\0';
            }
        }
        else
        {
            snprintf(name, size, "unknown");
        }
        fclose(f);
    }
    else
    {
        snprintf(name, size, "unknown");
    }
}

// Get client information
void get_client_info(int socket, connection_info *info)
{
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);

    getpeername(socket, (struct sockaddr *)&addr, &addr_len);
    getnameinfo((struct sockaddr *)&addr, addr_len,
                info->client_host, sizeof(info->client_host),
                info->client_service, sizeof(info->client_service),
                NI_NUMERICHOST | NI_NUMERICSERV);

    info->connect_time = time(NULL);

    // Try to get the client PID (Linux-specific)
    struct ucred ucred;
    socklen_t len = sizeof(struct ucred);
    if (getsockopt(socket, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == 0)
    {
        info->client_pid = ucred.pid;
    }
    else
    {
        info->client_pid = -1;
    }
}

// Print connection information
void print_connection_info(const connection_info *info, const char *event)
{
    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));

    char process_name[256] = "unknown";
    if (info->client_pid > 0)
    {
        get_process_name(info->client_pid, process_name, sizeof(process_name));
    }

    if (strcmp(event, "CLOSED") == 0)
    {
        printf("\n%s%s[%s] Connection %s%s\n", COLOR_BRIGHT, COLOR_RED, timestamp, event, COLOR_RESET);
    }
    else if (strcmp(event, "ESTABLISHED") == 0)
    {
        printf("\n%s%s[%s] Connection %s%s\n", COLOR_BRIGHT, COLOR_GREEN, timestamp, event, COLOR_RESET);
    }
    else
    {
        printf("\n%s%s[%s] Connection %s%s\n", COLOR_BRIGHT, COLOR_CYAN, timestamp, event, COLOR_RESET);
    }
    printf("├─ Host: %s:%s\n", info->client_host, info->client_service);
    printf("├─ Process: %s (PID: %d)\n", process_name, info->client_pid);

    char time_str[64];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", localtime(&info->connect_time));
    printf("└─ Time: %s\n\n", time_str);
}

// Try to pretty print if the data looks like JSON
void print_data(const char *direction, const char *data, ssize_t length)
{
    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));

    char *data_copy = (char *)malloc(length + 1);
    memcpy(data_copy, data, length);
    data_copy[length] = '\0';

    if (strcmp(direction, "LSP -> Compiler") == 0)
    {
        printf("\n%s%s[%s] %s%s:\n", COLOR_BRIGHT, COLOR_YELLOW, timestamp, direction, COLOR_RESET);
    }
    else
    {
        printf("\n%s%s[%s] %s%s:\n", COLOR_BRIGHT, COLOR_GREEN, timestamp, direction, COLOR_RESET);
    }

    if (data_copy[0] == '{' || data_copy[0] == '[')
    {
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

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if ((proxy_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Proxy socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(proxy_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = INADDR_ANY;
    proxy_addr.sin_port = htons(PROXY_PORT);

    if (bind(proxy_socket, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) < 0)
    {
        perror("Proxy bind failed");
        exit(EXIT_FAILURE);
    }

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
        if ((client_socket = accept(proxy_socket, (struct sockaddr *)&client_addr, &addr_len)) < 0)
        {
            if (errno == EINTR)
                continue;
            perror("Accept failed");
            break;
        }

        // Get and print connection information
        connection_info info;
        get_client_info(client_socket, &info);
        print_connection_info(&info, "ESTABLISHED");

        if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Server socket creation failed");
            close(client_socket);
            continue;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(ORIGINAL_PORT);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            perror("Connection to original server failed");
            close(client_socket);
            close(server_socket);
            continue;
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            close(proxy_socket);

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
                        continue;
                    perror("Select failed");
                    break;
                }

                if (FD_ISSET(client_socket, &readfds))
                {
                    ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
                    if (bytes_read <= 0)
                    {
                        print_connection_info(&info, "CLOSED");
                        break;
                    }
                    print_data("LSP -> Compiler", buffer, bytes_read);
                    send(server_socket, buffer, bytes_read, 0);
                }

                if (FD_ISSET(server_socket, &readfds))
                {
                    ssize_t bytes_read = recv(server_socket, buffer, BUFFER_SIZE, 0);
                    if (bytes_read <= 0)
                    {
                        print_connection_info(&info, "CLOSED");
                        break;
                    }
                    print_data("Compiler -> LSP", buffer, bytes_read);
                    send(client_socket, buffer, bytes_read, 0);
                }
            }

            close(client_socket);
            close(server_socket);
            exit(EXIT_SUCCESS);
        }

        close(client_socket);
        close(server_socket);
    }

    close(proxy_socket);
    printf("\n%sMonitor shutting down...%s\n", COLOR_BRIGHT, COLOR_RESET);
    return 0;
}