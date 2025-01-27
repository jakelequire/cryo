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
#include "diagnostics/diagnostics.hpp"
#include <vector>
#include <iostream>
#include <sstream>

namespace Cryo
{

    struct GlobalDiagnosticsManager::Impl
    {
        std::vector<DiagnosticEntry> errors;
        std::vector<ErrorCallback> callbacks;
        std::string buildDir;
    };

    DiagnosticEntry::DiagnosticEntry(CryoErrorCode code,
                                     CryoErrorSeverity severity,
                                     std::string message,
                                     std::string filename,
                                     int line,
                                     int column)
        : code(code), severity(severity), message(std::move(message)), filename(std::move(filename)), line(line), column(column) {}

    GlobalDiagnosticsManager::GlobalDiagnosticsManager(const char *buildDir)
        : pImpl(new Impl)
    {
        if (buildDir)
        {
            pImpl->buildDir = buildDir;
        }
    }

    GlobalDiagnosticsManager::~GlobalDiagnosticsManager() = default;

    void GlobalDiagnosticsManager::reportError(CryoErrorCode code,
                                               CryoErrorSeverity severity,
                                               const std::string &message,
                                               const std::string &filename,
                                               int line,
                                               int column)
    {
        DiagnosticEntry entry(code, severity, message, filename, line, column);
        pImpl->errors.push_back(entry);

        // Notify callbacks
        for (const auto &callback : pImpl->callbacks)
        {
            callback(entry);
        }
    }

    void GlobalDiagnosticsManager::registerErrorCallback(ErrorCallback callback)
    {
        pImpl->callbacks.push_back(std::move(callback));
    }

    bool GlobalDiagnosticsManager::hasErrors() const
    {
        return !pImpl->errors.empty();
    }

    size_t GlobalDiagnosticsManager::getErrorCount() const
    {
        return pImpl->errors.size();
    }

    void GlobalDiagnosticsManager::clearErrors()
    {
        pImpl->errors.clear();
    }

    std::string GlobalDiagnosticsManager::formatError(const DiagnosticEntry &entry) const
    {
        std::stringstream ss;

        // Severity prefix
        switch (entry.getSeverity())
        {
        case CRYO_SEVERITY_NOTE:
            ss << "note: ";
            break;
        case CRYO_SEVERITY_WARNING:
            ss << "warning: ";
            break;
        case CRYO_SEVERITY_ERROR:
            ss << "error: ";
            break;
        case CRYO_SEVERITY_FATAL:
            ss << "fatal error: ";
            break;
        }

        // Location information
        if (!entry.getFilename().empty())
        {
            ss << entry.getFilename();
            if (entry.getLine() > 0)
            {
                ss << ":" << entry.getLine();
                if (entry.getColumn() > 0)
                {
                    ss << ":" << entry.getColumn();
                }
            }
            ss << ": ";
        }

        // Error message and code
        ss << entry.getMessage() << " [" << entry.getCode() << "]";

        return ss.str();
    }

    void GlobalDiagnosticsManager::printErrors() const
    {
        for (const auto &error : pImpl->errors)
        {
            std::cerr << formatError(error) << std::endl;
        }
    }

    // C API Implementation
    extern "C"
    {

        CryoDiagnostics *CryoDiagnostics_Create(const char *buildDir)
        {
            try
            {
                auto *manager = new GlobalDiagnosticsManager(buildDir);
                return reinterpret_cast<CryoDiagnostics *>(manager);
            }
            catch (...)
            {
                return nullptr;
            }
        }

        void CryoDiagnostics_Destroy(CryoDiagnostics *diagnostics)
        {
            if (diagnostics)
            {
                auto *manager = reinterpret_cast<GlobalDiagnosticsManager *>(diagnostics);
                delete manager;
            }
        }

        void CryoDiagnostics_ReportError(CryoDiagnostics *diagnostics,
                                         CryoErrorCode code,
                                         CryoErrorSeverity severity,
                                         const char *message,
                                         const char *filename,
                                         int line,
                                         int column)
        {
            if (diagnostics && message)
            {
                auto *manager = reinterpret_cast<GlobalDiagnosticsManager *>(diagnostics);
                manager->reportError(code, severity, message,
                                     filename ? filename : "", line, column);
            }
        }

        int CryoDiagnostics_HasErrors(CryoDiagnostics *diagnostics)
        {
            if (diagnostics)
            {
                auto *manager = reinterpret_cast<GlobalDiagnosticsManager *>(diagnostics);
                return manager->hasErrors() ? 1 : 0;
            }
            return 0;
        }

        int CryoDiagnostics_GetErrorCount(CryoDiagnostics *diagnostics)
        {
            if (diagnostics)
            {
                auto *manager = reinterpret_cast<GlobalDiagnosticsManager *>(diagnostics);
                return static_cast<int>(manager->getErrorCount());
            }
            return 0;
        }

    } // extern "C"

} // namespace Cryo
