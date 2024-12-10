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
#include "tools/cxx/logger.hpp"

namespace Cryo
{

    Logger::Logger(LogLevel level)
        : currentLevel(level), useConsole(true), logFile(nullptr) {}

    Logger::~Logger()
    {
        if (logFile)
        {
            logFile->close();
        }
    }

    void Logger::setLogLevel(LogLevel level)
    {
        currentLevel = level;
    }

    void Logger::setLogFile(const std::string &filename)
    {
        logFile = std::make_unique<std::ofstream>(filename, std::ios::app);
        if (!logFile->is_open())
        {
            std::cerr << "Failed to open log file: " << filename << std::endl;
            logFile = nullptr;
        }
    }

    void Logger::enableConsoleOutput(bool enable)
    {
        useConsole = enable;
    }

    void Logger::logMessage(LogLevel level, const char *message)
    {
        if (level < currentLevel)
            return;

        // Get current time
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;

        std::stringstream ss;
        ss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
           << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
           << "[" << getLevelString(level) << "] "
           << message;

        if (useConsole)
        {
            std::cout << ss.str() << std::endl;
        }

        if (logFile && logFile->is_open())
        {
            *logFile << ss.str() << std::endl;
            logFile->flush();
        }
    }

    const char *Logger::getLevelString(LogLevel level)
    {
        switch (level)
        {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARN";
        case ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

} // namespace Cryo