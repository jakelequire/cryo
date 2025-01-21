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
#ifndef GLOBAL_DIAGNOSTICS_MANAGER_H
#define GLOBAL_DIAGNOSTICS_MANAGER_H

#include "errorCodes.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

// ================================================================ //
// C Interface
#ifdef __cplusplus
extern "C"
{
#endif

    // Opaque pointer for C API
    typedef struct CryoDiagnostics_t *CryoDiagnostics;

    // Constructor & Destructor
    CryoDiagnostics *CryoDiagnostics_Create(const char *buildDir);
    void CryoDiagnostics_Destroy(CryoDiagnostics *diagnostics);

    // Error reporting
    void CryoDiagnostics_ReportError(CryoDiagnostics *diagnostics,
                                     CryoErrorCode code,
                                     CryoErrorSeverity severity,
                                     const char *message,
                                     const char *filename,
                                     int line,
                                     int column);

    // Error querying
    int CryoDiagnostics_HasErrors(CryoDiagnostics *diagnostics);
    int CryoDiagnostics_GetErrorCount(CryoDiagnostics *diagnostics);

#ifdef __cplusplus
}
#endif

namespace Cryo
{

    // Forward declarations
    class DiagnosticEntry;

    class GlobalDiagnosticsManager
    {
    public:
        GlobalDiagnosticsManager(const char *buildDir);
        ~GlobalDiagnosticsManager();

        // Error reporting methods
        void reportError(CryoErrorCode code,
                         CryoErrorSeverity severity,
                         const std::string &message,
                         const std::string &filename = "",
                         int line = -1,
                         int column = -1);

        // Error callback registration
        using ErrorCallback = std::function<void(const DiagnosticEntry &)>;
        void registerErrorCallback(ErrorCallback callback);

        // Query methods
        bool hasErrors() const;
        size_t getErrorCount() const;
        void clearErrors();

        // Format and output methods
        std::string formatError(const DiagnosticEntry &entry) const;
        void printErrors() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> pImpl; // PIMPL idiom to hide implementation details

        // Disable copy and assignment
        GlobalDiagnosticsManager(const GlobalDiagnosticsManager &) = delete;
        GlobalDiagnosticsManager &operator=(const GlobalDiagnosticsManager &) = delete;
    };

    // Represents a single diagnostic entry
    class DiagnosticEntry
    {
    public:
        DiagnosticEntry(CryoErrorCode code,
                        CryoErrorSeverity severity,
                        std::string message,
                        std::string filename = "",
                        int line = -1,
                        int column = -1);

        CryoErrorCode getCode() const { return code; }
        CryoErrorSeverity getSeverity() const { return severity; }
        const std::string &getMessage() const { return message; }
        const std::string &getFilename() const { return filename; }
        int getLine() const { return line; }
        int getColumn() const { return column; }

    private:
        CryoErrorCode code;
        CryoErrorSeverity severity;
        std::string message;
        std::string filename;
        int line;
        int column;
    };

} // namespace Cryo

#endif // GLOBAL_DIAGNOSTICS_MANAGER_H
