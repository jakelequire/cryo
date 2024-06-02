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
#include "runtime.h"


void cryo_runtime(void) {
    // Adjust the build command as needed
    const char* build_command = "make all"; // e.g., "make", "gcc main.c -o output"

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    SECURITY_ATTRIBUTES sa;
    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;
    HANDLE g_hChildStd_ERR_Rd = NULL;
    HANDLE g_hChildStd_ERR_Wr = NULL;
    DWORD dwRead;
    CHAR chBuf[4096];
    BOOL bSuccess = FALSE;

    // Set up the security attributes struct.
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT.
    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0)) {
        printf("StdoutRd CreatePipe failed\n");
        return;
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
        printf("Stdout SetHandleInformation failed\n");
        return;
    }

    // Create a pipe for the child process's STDERR.
    if (!CreatePipe(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr, &sa, 0)) {
        printf("StderrRd CreatePipe failed\n");
        return;
    }

    // Ensure the read handle to the pipe for STDERR is not inherited.
    if (!SetHandleInformation(g_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0)) {
        printf("Stderr SetHandleInformation failed\n");
        return;
    }

    // Set up members of the PROCESS_INFORMATION structure.
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure.
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.hStdError = g_hChildStd_ERR_Wr;
    si.hStdOutput = g_hChildStd_OUT_Wr;
    si.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process.
    bSuccess = CreateProcess(NULL,
        (LPSTR)build_command,       // command line
        NULL,                       // process security attributes
        NULL,                       // primary thread security attributes
        TRUE,                       // handles are inherited
        0,                          // creation flags
        NULL,                       // use parent's environment
        NULL,                       // use parent's current directory
        &si,                        // STARTUPINFO pointer
        &pi                         // receives PROCESS_INFORMATION
    );

    // If an error occurs, exit the application.
    if (!bSuccess) {
        printf("CreateProcess failed\n");
        return;
    } else {
        // Close handles to the child process and its primary thread.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        // Close the write end of the pipes before reading from the read end of the pipes.
        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_ERR_Wr);

        // Read output from the child process's pipe for STDOUT
        // and write to the parent process's pipe for STDOUT.
        // Stop when there is no more data.
        for (;;) {
            bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, sizeof(chBuf), &dwRead, NULL);
            if (!bSuccess || dwRead == 0) break;
            fwrite(chBuf, 1, dwRead, stdout);
        }

        // Read output from the child process's pipe for STDERR
        // and write to the parent process's pipe for STDERR.
        // Stop when there is no more data.
        for (;;) {
            bSuccess = ReadFile(g_hChildStd_ERR_Rd, chBuf, sizeof(chBuf), &dwRead, NULL);
            if (!bSuccess || dwRead == 0) break;
            fwrite(chBuf, 1, dwRead, stderr);
        }

        // Close the read end of the pipes.
        CloseHandle(g_hChildStd_OUT_Rd);
        CloseHandle(g_hChildStd_ERR_Rd);
    }

    return;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }
    cryo_runtime();
    return 0;
}
