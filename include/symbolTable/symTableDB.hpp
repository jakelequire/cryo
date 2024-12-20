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
#ifndef SYMTABLE_DB_H
#define SYMTABLE_DB_H

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <map>

#include "symbolTable/symdefs.h"

/*
DB Structure
============================================================

Function Table:
X        [0]        [1]     [2]    [3]    [4]       [5]        [6]          [7]
[n] Typeof Symbol   Name    Type   ID    Depth   Arg Count  [Arg Types]  Visibility
[...]
----------------------------------------------------------------------------

Variable Table:
X        [0]        [1]     [2]    [3]    [4]       [5]
[n] Typeof Symbol   Name    Type   ID    Depth   Visibility
[...]
----------------------------------------------------------------------------

Type Table:
X        [0]        [1]     [2]    [3]    [4]        [5]            [6]           [7]
[n] Typeof Symbol   Name    Type   ID    Depth   Param Count   [Param Types]   Visibility
[...]
----------------------------------------------------------------------------
*/

namespace Cryo
{

    struct DBDirectories
    {
        std::string buildDir;
        std::string dependencyDir;
        std::string debugDir;
        std::string DBdir;
    };

    enum class TableType
    {
        FUNCTION_TABLE,
        VARIABLE_TABLE,
        TYPE_TABLE
    };

    struct TableEntry
    {
        std::vector<std::string> columns;
        TableType type;
    };

    class SymbolTableDB
    {
    public:
        SymbolTableDB(std::string rootDir);
        ~SymbolTableDB();

        // New primary interface for serializing a SymbolTable
        bool serializeSymbolTable(const SymbolTable *table);
        bool appendSerializedTable(const SymbolTable *table);
        bool deserializeToSymbolTable(SymbolTable *table);
        bool createScopedDB() const;

        // Table operations
        bool createTable(TableType type);
        bool dropTable(TableType type);

        // Row operations
        bool insertRow(TableType type, const TableEntry &entry);
        bool deleteRow(TableType type, size_t rowId);
        bool updateRow(TableType type, size_t rowId, const TableEntry &entry);

        // Query operations
        std::vector<TableEntry> queryTable(TableType type) const;
        TableEntry queryRow(TableType type, size_t rowId);
        std::vector<TableEntry> queryByColumn(TableType type, size_t columnIndex, const std::string &value);

        void addSingleSymbolToTable(Symbol *symbol, SymbolTable *table);

        TypeOfSymbol stringToSymbolType(const std::string &type) const;

    private:
        DBDirectories dirs;

        // File handles for each table
        FILE *functionTable;
        FILE *variableTable;
        FILE *typeTable;

        // Table metadata
        const size_t FUNCTION_TABLE_COLS = 8;
        const size_t VARIABLE_TABLE_COLS = 6;
        const size_t TYPE_TABLE_COLS = 8;

        void initDirectories(std::string rootDir);
        void initTables();

        // Symbol parsing and conversion
        TableEntry symbolToFunctionEntry(const Symbol *symbol) const;
        TableEntry symbolToVariableEntry(const Symbol *symbol) const;
        TableEntry symbolToTypeEntry(const Symbol *symbol) const;

        Symbol *entryToSymbol(const TableEntry &entry) const;

        // File operations
        bool writeEntry(FILE *file, const TableEntry &entry);
        TableEntry readEntry(FILE *file) const;
        bool seekToRow(FILE *file, size_t rowId);

        // Validation
        bool validateEntry(TableType type, const TableEntry &entry) const;
        size_t getColumnCount(TableType type) const;

        // File paths
        std::string getTablePath(TableType type) const;

        // Utility functions
        void serializeEntry(const TableEntry &entry, std::string &output) const;
        TableEntry deserializeEntry(const std::string &input) const;
        std::string symbolTypeToString(TypeOfSymbol type) const;
        std::string visibilityToString(CryoVisibilityType visibility) const;

        // Table file management
        FILE *openTable(TableType type, const char *mode) const;
        void closeTable(FILE *file) const;

        // Entry Conversion
        Symbol *functionEntryToSymbol(const TableEntry &entry) const;
        Symbol *variableEntryToSymbol(const TableEntry &entry) const;
        Symbol *typeEntryToSymbol(const TableEntry &entry) const;

        // Box drawing characters
        const char *_TOP_LEFT = "+";
        const char *_TOP_RIGHT = "+";
        const char *_BOTTOM_LEFT = "+";
        const char *_BOTTOM_RIGHT = "+";
        const char *_HORIZONTAL = "-";
        const char *_VERTICAL = "|";
        const char *_T_DOWN = "+";
        const char *_T_UP = "+";
        const char *_T_RIGHT = "+";
        const char *_T_LEFT = "+";
        const char *_CROSS = "+";

        // Add these utility functions
        void writeTableHeader(FILE *file, TableType type) const;
        std::string getTableHeader(TableType type) const;
        void writeTableFooter(FILE *file) const;

        // Add struct for column formatting
        struct ColumnFormat
        {
            std::string name;
            size_t width;
        };
        std::vector<ColumnFormat> getColumnFormats(TableType type) const;
        const std::vector<size_t> getColumnWidths(const std::vector<ColumnFormat> &formats) const;

        struct ChunkEntry
        {
            TableEntry functionEntry;
            std::vector<TableEntry> variables;
        };

        std::string getChunkPath() const;
        void writeChunkHeader(FILE *file) const;
        void writeChunkEntry(FILE *file, const ChunkEntry &chunk) const;
        std::vector<ChunkEntry> groupEntriesByID() const;
        bool validateChunkEntry(const ChunkEntry &chunk) const;
    };
}

#endif // SYMTABLE_DB_H
