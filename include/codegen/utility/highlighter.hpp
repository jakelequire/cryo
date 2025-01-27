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
#pragma once

#include <string>
#include <regex>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <numeric>
#include <iostream>
#include <functional>

#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/IR/Module.h"

class LLVMIRHighlighter : public llvm::AssemblyAnnotationWriter
{
private:
    // ANSI escape codes for colors
    struct Colors
    {
        static constexpr const char *RESET = "\033[0m";
        static constexpr const char *INSTRUCTION = "\033[38;5;141m"; // Light purple for instructions
        static constexpr const char *MEMORY = "\033[1;38;5;208m";    // Orange & bold for memory operations
        static constexpr const char *TYPE = "\033[38;5;81m";         // Light blue for types
        static constexpr const char *KEYWORD = "\033[38;5;208m";     // Orange for keywords
        static constexpr const char *FUNCTION = "\033[38;5;222m";    // Light yellow for functions
        static constexpr const char *VARIABLE = "\033[38;5;121m";    // Light green for variables
        static constexpr const char *NUMBER = "\033[38;5;223m";      // Light yellow for numbers
        static constexpr const char *STRING = "\033[38;5;216m";      // Light red for strings
        static constexpr const char *COMMENT = "\033[38;5;242m";     // Gray for comments
        static constexpr const char *ATTRIBUTE = "\033[38;5;146m";   // Light gray for attributes
        static constexpr const char *LABEL = "\033[38;5;159m";       // Light cyan for labels
        static constexpr const char *OPERATOR = "\033[38;5;231m";    // White for operators
        static constexpr const char *BLOCK = "\033[35m";             // Purple for basic blocks
        static constexpr const char *ERROR = "\033[1;31m";           // Dark red for errors
    };

    struct TokenPattern
    {
        std::regex pattern;
        const char *color;
        int priority;
    };

    std::vector<TokenPattern> patterns;

    std::vector<std::string> LLVM_ERROR_MESSAGES = {
        "Function return type does not match operand type of return inst!",
        "GEP base pointer is not a vector or a vector of pointers",
        "Load operand must be a pointer.",
        "Call parameter type does not match function signature!",
        "Function context does not match Module context!",
        "Instruction does not dominate all uses!",
    };

    void initializePatterns()
    {
        // Higher priority patterns are processed first
        patterns = {
            // Error messages (Using the LLVM_ERROR_MESSAGES vector) with iterator
            {std::regex("(" + std::accumulate(LLVM_ERROR_MESSAGES.begin(), LLVM_ERROR_MESSAGES.end(), std::string(""), [](std::string &a, std::string &b)
                                              { return a + (a.length() > 0 ? "|" : "") + b; }) +
                        ")"),
             Colors::ERROR, 1100},

            // Comments
            {std::regex(";.*$"), Colors::COMMENT, 1000},

            // Strings
            {std::regex("\"[^\"]*\""), Colors::STRING, 900},

            // Basic blocks
            // {std::regex("^[a-zA-Z][a-zA-Z0-9._]*:", std::regex::multiline), Colors::BLOCK, 850},

            // Function declarations
            {std::regex("(define|declare)\\s+([^@]*)(@[\\w._-]+)"), Colors::KEYWORD, 800},

            // Common LLVM instructions with type handling
            {std::regex("\\b(add|sub|mul|div|rem|and|or|xor|shl|lshr|ashr|fadd|fsub|fmul|fdiv|frem)\\b"), Colors::INSTRUCTION, 700},
            {std::regex("\\b(icmp|fcmp|phi|call|ret|br|switch)\\b"), Colors::INSTRUCTION, 700},

            // Memory operations
            {std::regex("\\b(alloca|load|store|getelementptr|inbounds)\\b"), Colors::MEMORY, 600},

            // Type patterns (handle array types properly)
            {std::regex("\\b(ptr|void|i1|i8|i16|i32|i64|float|double|x86_fp80|fp128|ppc_fp128|label|metadata)\\b"), Colors::TYPE, 600},
            {std::regex("\\[\\s*\\d+\\s*x\\s*[\\w\\d*]+\\]"), Colors::TYPE, 600},

            // Attributes and alignment
            {std::regex("\\balign\\s+\\d+\\b"), Colors::ATTRIBUTE, 500},
            {std::regex("\\b(nonnull|dereferenceable|align|inbounds|volatile)\\b"), Colors::ATTRIBUTE, 500},

            // Numbers (including negative and floating point)
            {std::regex("-?\\b\\d+\\.?\\d*\\b"), Colors::NUMBER, 400},

            // Variables and labels (but not type suffixes)
            {std::regex("%[\\w\\d.]+(?!\\.(?:ptr|load|store|call))\\b"), Colors::VARIABLE, 300},
            {std::regex("@[\\w\\d.]+(?!\\.(?:ptr|load|store|call))\\b"), Colors::FUNCTION, 300},

            // Comparison operators
            {std::regex("\\b(eq|ne|ugt|uge|ult|ule|sgt|sge|slt|sle|oeq|one|olt|ole|ogt|oge|ord|uno|ueq|une)\\b"), Colors::OPERATOR, 200},

            // Basic operators
            {std::regex("[=<>!+\\-*/%&|^]+"), Colors::OPERATOR, 100},
        };
    }

public:
    LLVMIRHighlighter()
    {
        initializePatterns();
    }

    virtual bool isVerbose() const { return false; }

    void emitInstructionAnnot(const llvm::Instruction *instr, llvm::formatted_raw_ostream &os) override
    {
        // Don't print the instruction again, just add color to what's already been printed
        os << "\n"; // Add newline for proper formatting
    }

    // New method to handle pre-printing coloring
    void printWithHighlighting(llvm::Module *module, llvm::formatted_raw_ostream &os)
    {
        std::string moduleStr;
        llvm::raw_string_ostream ss(moduleStr);
        module->print(ss, nullptr);
        ss.flush();

        std::string highlighted = highlightText(moduleStr);
        os << highlighted;
    }

    std::string highlightText(const std::string &text)
    {
        // Create a vector of tokens with their positions and colors
        struct Token
        {
            size_t start;
            size_t end;
            const char *color;
            int priority;
        };
        std::vector<Token> tokens;

        // Find all matches for each pattern
        for (const auto &pattern : patterns)
        {
            std::sregex_iterator it(text.begin(), text.end(), pattern.pattern);
            std::sregex_iterator end;

            while (it != end)
            {
                std::smatch match = *it;
                tokens.push_back({static_cast<size_t>(match.position()),
                                  static_cast<size_t>(match.position() + match.length()),
                                  pattern.color,
                                  pattern.priority});
                ++it;
            }
        }

        // Sort tokens by position and priority
        std::sort(tokens.begin(), tokens.end(),
                  [](const Token &a, const Token &b)
                  {
                      if (a.start != b.start)
                          return a.start < b.start;
                      return a.priority > b.priority;
                  });

        // Apply colors while handling overlaps
        std::string result = text;
        size_t offset = 0;
        std::vector<size_t> activeRegions;

        for (const auto &token : tokens)
        {
            size_t start = token.start + offset;
            size_t end = token.end + offset;

            // Check if this token overlaps with higher priority ones
            bool overlap = false;
            for (size_t pos : activeRegions)
            {
                if (start < pos)
                {
                    overlap = true;
                    break;
                }
            }

            if (!overlap)
            {
                result.insert(end, Colors::RESET);
                result.insert(start, token.color);
                offset += strlen(token.color) + strlen(Colors::RESET);
                activeRegions.push_back(end + strlen(token.color) + strlen(Colors::RESET));
            }
        }

        return result;
    }
};
