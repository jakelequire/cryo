#include "runtime.h"
#define DEBOUNCE_TIME 3

time_t last_compile_time = 0;

// Array of directories to ignore
const char *ignore_dirs[] = {"./src/bin"};
const int ignore_dirs_count = sizeof(ignore_dirs) / sizeof(ignore_dirs[0]);

// <compile_and_reload>
void compile_and_reload() {
    // Command to compile the project
    system("make");

    // Logic to reload the relevant components
    // This can include dynamic unloading and loading of shared libraries
}
// </compile_and_reload>

bool should_ignore_event(const char *event_name) {
    for (int i = 0; i < ignore_dirs_count; i++) {
        if (strncmp(event_name, ignore_dirs[i], strlen(ignore_dirs[i])) == 0) {
            return true;
        }
    }
    return false;
}

// <handle_event>
void handle_event(const char *event_name) {
    if (should_ignore_event(event_name)) {
        // Ignore events from the specified directories
        return;
    }

    // Get the current time
    time_t current_time;
    time(&current_time);

    // Check if the debounce time has passed
    if (difftime(current_time, last_compile_time) >= DEBOUNCE_TIME) {
        printf("File %s changed, recompiling...\n", event_name);
        compile_and_reload();
        last_compile_time = current_time; // Update the last compile time
    }
}
// </handle_event>

#ifdef __linux__
// <watch_files> (Linux only)
void* watch_files(void* arg) {
    int length, i = 0;
    int fd;
    int wd;
    char buffer[EVENT_BUF_LEN];

    // Initialize inotify
    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        pthread_exit(NULL);
    }

    // Add watch for the current directory
    wd = inotify_add_watch(fd, ".", IN_MODIFY | IN_CREATE | IN_DELETE);
    if (wd == -1) {
        perror("inotify_add_watch");
        pthread_exit(NULL);
    }

    while (1) {
        // Read the events
        length = read(fd, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            perror("read");
            pthread_exit(NULL);
        }

        // Process the events
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len) {
                if (event->mask & IN_MODIFY || event->mask & IN_CREATE || event->mask & IN_DELETE) {
                    handle_event(event->name);
                }
            }
            i += EVENT_SIZE + event->len;
        }
        i = 0;
    }

    // Clean up
    inotify_rm_watch(fd, wd);
    close(fd);
    pthread_exit(NULL);
}
// </watch_files> (Linux only)

#elif defined(_WIN32) || defined(_WIN64)
// <watch_files> (Windows only)
unsigned __stdcall watch_files(void* arg) {
    HANDLE hDir = CreateFile(
        ".",                // Directory to watch
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (hDir == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateFile failed with error: %ld\n", GetLastError());
        return 1;
    }

    char buffer[1024];
    DWORD bytesReturned;
    FILE_NOTIFY_INFORMATION *pNotify;
    char filename[MAX_PATH];
    
    while (1) {
        if (ReadDirectoryChangesW(
            hDir,
            &buffer,
            sizeof(buffer),
            TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
            &bytesReturned,
            NULL,
            NULL) == 0) {
            fprintf(stderr, "ReadDirectoryChangesW failed with error: %ld\n", GetLastError());
            break;
        }

        int offset = 0;
        do {
            pNotify = (FILE_NOTIFY_INFORMATION *) &buffer[offset];
            int length = WideCharToMultiByte(CP_ACP, 0, pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR), filename, sizeof(filename) - 1, NULL, NULL);
            filename[length] = '\0';
            handle_event(filename);
            offset += pNotify->NextEntryOffset;
        } while (pNotify->NextEntryOffset != 0);
    }

    CloseHandle(hDir);
    return 0;
}
// </watch_files> (Windows only)
#endif

#ifdef __linux__
// <read_user_input>
void* read_user_input(void* arg) {
    char command[256];

    while (1) {
        printf("> ");
        if (fgets(command, sizeof(command), stdin) != NULL) {
            // Remove trailing newline character
            command[strcspn(command, "\n")] = 0;

            // Execute the command
            if (system(command) == -1) {
                perror("system");
            }
        }
    }

    pthread_exit(NULL);
}
// </read_user_input>
#elif defined(_WIN32) || defined(_WIN64)
unsigned __stdcall read_user_input(void* arg) {
    char command[256];

    while (1) {
        printf("> ");
        if (fgets(command, sizeof(command), stdin) != NULL) {
            // Remove trailing newline character
            command[strcspn(command, "\n")] = 0;

            // Execute the command
            if (system(command) == -1) {
                perror("system");
            }
        }
    }

    return 0;
}
#endif

void terminate_running_instance(const char *process_name) {
#ifdef __linux__
    // Linux implementation (if needed)
#elif defined(_WIN32) || defined(_WIN64)
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return;
    }

    do {
        if (strcmp(pe32.szExeFile, process_name) == 0) {
            hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if (hProcess != NULL) {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
#endif
}

// <cryo_runtime>
void cryo_runtime(void) {
    printf("Entering Cryo environment...\n");

#ifdef __linux__
    // Linux-specific code remains unchanged
    pthread_t watcher_thread, input_thread;

    // Create threads for file watching and user input
    if (pthread_create(&watcher_thread, NULL, watch_files, NULL) != 0) {
        perror("pthread_create (watcher_thread)");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&input_thread, NULL, read_user_input, NULL) != 0) {
        perror("pthread_create (input_thread)");
        exit(EXIT_FAILURE);
    }

    // Wait for threads to finish
    pthread_join(watcher_thread, NULL);
    pthread_join(input_thread, NULL);
#elif defined(_WIN32) || defined(_WIN64)
    HANDLE watcher_thread, input_thread;
    unsigned watcher_thread_id, input_thread_id;

    // Terminate any running instance of runtime.exe before starting a new one
    terminate_running_instance("runtime.exe");

    // Create thread for file watching
    watcher_thread = (HANDLE)_beginthreadex(NULL, 0, watch_files, NULL, 0, &watcher_thread_id);
    if (watcher_thread == 0) {
        perror("_beginthreadex (watcher_thread)");
        exit(EXIT_FAILURE);
    }

    // Create thread for user input
    input_thread = (HANDLE)_beginthreadex(NULL, 0, read_user_input, NULL, 0, &input_thread_id);
    if (input_thread == 0) {
        perror("_beginthreadex (input_thread)");
        exit(EXIT_FAILURE);
    }

    // Wait for threads to finish
    WaitForSingleObject(watcher_thread, INFINITE);
    WaitForSingleObject(input_thread, INFINITE);

    CloseHandle(watcher_thread);
    CloseHandle(input_thread);
#endif
}
// </cryo_runtime>

// <main>
int main(int argc, char** argv) {
    cryo_runtime();
    return 0;
}
// </main>
