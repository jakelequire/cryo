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
#ifndef CRYO_LOGGER_H
#define CRYO_LOGGER_H

#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <ctime>
#include <iomanip>
#include <chrono>

namespace Cryo
{

    class Logger
    {
    public:
        enum LogLevel
        {
            DEBUG,
            INFO,
            WARNING,
            ERROR
        };

        Logger(LogLevel level = INFO);
        ~Logger();

        void setLogLevel(LogLevel level);
        void setLogFile(const std::string &filename);
        void enableConsoleOutput(bool enable);

        template <typename... Args>
        void log(const char *format, Args... args)
        {
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), format, args...);
            logMessage(INFO, buffer);
        }

        template <typename... Args>
        void debugNode(const char *format, Args... args)
        {
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), format, args...);
            logNode(DEBUG, buffer);
        }

        template <typename... Args>
        void debug(const char *format, Args... args)
        {
            if (currentLevel <= DEBUG)
            {
                char buffer[1024];
                snprintf(buffer, sizeof(buffer), format, args...);
                logMessage(DEBUG, buffer);
            }
        }

        template <typename... Args>
        void warning(const char *format, Args... args)
        {
            if (currentLevel <= WARNING)
            {
                char buffer[1024];
                snprintf(buffer, sizeof(buffer), format, args...);
                logMessage(WARNING, buffer);
            }
        }

        template <typename... Args>
        void error(const char *format, Args... args)
        {
            if (currentLevel <= ERROR)
            {
                char buffer[1024];
                snprintf(buffer, sizeof(buffer), format, args...);
                logMessage(ERROR, buffer);
            }
        }

    private:
        void logMessage(LogLevel level, const char *message);
        void logNode(LogLevel level, const char *message);
        const char *getLevelString(LogLevel level);

        LogLevel currentLevel;
        bool useConsole;
        std::unique_ptr<std::ofstream> logFile;
    };

} // namespace Cryo

#endif // CRYO_LOGGER_H