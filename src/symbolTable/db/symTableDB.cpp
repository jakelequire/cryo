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
#include "symbolTable/symTableDB.hpp"

#define READ_BUFFER_SIZE 1024 * 1024 // 1MB

namespace Cryo
{
    SymbolTableDB::SymbolTableDB(std::string rootDir)
    {
        std::cout << "Initializing Symbol Table DB with path: " << rootDir << std::endl;
        initDirectories(rootDir);
        initTables();

        functionTable = nullptr;
        variableTable = nullptr;
        typeTable = nullptr;

        std::cout << "Symbol Table DB initialized" << std::endl;
    }

    SymbolTableDB::~SymbolTableDB()
    {
        if (functionTable)
            closeTable(functionTable);
        if (variableTable)
            closeTable(variableTable);
        if (typeTable)
            closeTable(typeTable);
    }

    void SymbolTableDB::initDirectories(std::string rootDir)
    {
        // Set directory paths
        dirs.buildDir = rootDir + "build";
        dirs.dependencyDir = dirs.buildDir + "/out/deps";
        dirs.debugDir = dirs.buildDir + "/debug";
        dirs.DBdir = dirs.buildDir + "/db";

        // Helper function to safely create directory
        auto createDirIfNeeded = [](const std::string &path)
        {
            try
            {
                if (std::filesystem::exists(path))
                {
                    if (!std::filesystem::is_directory(path))
                    {
                        std::cerr << "Warning: Path exists but is not a directory: " << path << std::endl;
                        return false;
                    }
                    return true;
                }

                return std::filesystem::create_directories(path);
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                std::cerr << "Warning: Filesystem error for path " << path << ": " << e.what() << std::endl;
                return false;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Warning: Error handling path " << path << ": " << e.what() << std::endl;
                return false;
            }
        };

        // Create directories in hierarchical order
        std::vector<std::reference_wrapper<const std::string>> dirPaths = {
            dirs.buildDir,
            dirs.dependencyDir,
            dirs.debugDir,
            dirs.DBdir};

        for (const std::string &dir : dirPaths)
        {
            createDirIfNeeded(dir);
        }
    }

    void SymbolTableDB::initTables()
    {
        createTable(TableType::FUNCTION_TABLE);
        createTable(TableType::VARIABLE_TABLE);
        createTable(TableType::TYPE_TABLE);
    }

    bool SymbolTableDB::createTable(TableType type)
    {
        FILE *file = openTable(type, "w+");
        if (!file)
            return false;

        writeTableHeader(file, type);
        closeTable(file);
        return true;
    }

    bool SymbolTableDB::serializeSymbolTable(const SymbolTable *table)
    {
        if (!table || !table->symbols)
            return false;

        // Clear existing tables
        dropTable(TableType::FUNCTION_TABLE);
        dropTable(TableType::VARIABLE_TABLE);
        dropTable(TableType::TYPE_TABLE);

        // Create fresh tables
        createTable(TableType::FUNCTION_TABLE);
        createTable(TableType::VARIABLE_TABLE);
        createTable(TableType::TYPE_TABLE);

        // Process each symbol and write to appropriate table
        for (size_t i = 0; i < table->count; i++)
        {
            const Symbol *symbol = table->symbols[i];
            if (!symbol)
                continue;

            TableEntry entry;
            switch (symbol->symbolType)
            {
            case FUNCTION_SYMBOL:
                entry = symbolToFunctionEntry(symbol);
                if (!entry.columns.empty())
                {
                    insertRow(TableType::FUNCTION_TABLE, entry);
                }
                break;

            case VARIABLE_SYMBOL:
                entry = symbolToVariableEntry(symbol);
                if (!entry.columns.empty())
                {
                    insertRow(TableType::VARIABLE_TABLE, entry);
                }
                break;

            case TYPE_SYMBOL:
                entry = symbolToTypeEntry(symbol);
                if (!entry.columns.empty())
                {
                    insertRow(TableType::TYPE_TABLE, entry);
                }
                break;

            default:
                // Skip other symbol types
                break;
            }
        }

        return true;
    }

    bool SymbolTableDB::appendSerializedTable(const SymbolTable *table)
    {
        if (!table || !table->symbols)
            return false;

        // Process each symbol and write to appropriate table
        for (size_t i = 0; i < table->count; i++)
        {
            const Symbol *symbol = table->symbols[i];
            if (!symbol)
                continue;

            TableEntry entry;
            switch (symbol->symbolType)
            {
            case FUNCTION_SYMBOL:
                entry = symbolToFunctionEntry(symbol);
                if (!entry.columns.empty())
                {
                    insertRow(TableType::FUNCTION_TABLE, entry);
                }
                break;

            case VARIABLE_SYMBOL:
                entry = symbolToVariableEntry(symbol);
                if (!entry.columns.empty())
                {
                    insertRow(TableType::VARIABLE_TABLE, entry);
                }
                break;

            case TYPE_SYMBOL:
                entry = symbolToTypeEntry(symbol);
                if (!entry.columns.empty())
                {
                    insertRow(TableType::TYPE_TABLE, entry);
                }
                break;

            default:
                // Skip other symbol types
                break;
            }
        }

        return true;
    }

    TableEntry SymbolTableDB::symbolToFunctionEntry(const Symbol *symbol) const
    {
        TableEntry entry;
        if (!symbol || !symbol->function)
            return entry;

        const FunctionSymbol *func = symbol->function;

        // Build argument types string
        ASTNode *functionNode = func->node;
        const char *funcArgsStr = getFunctionArgTypeArrayStr(functionNode);
        std::stringstream argTypes = std::stringstream(funcArgsStr);

        entry.columns = {
            symbolTypeToString(symbol->symbolType),        // Symbol type
            func->name,                                    // Name
            DataTypeToStringUnformatted(func->returnType), // Return type
            func->functionScopeId,                         // ID
            std::to_string(0),                             // Scope depth
            std::to_string(func->paramCount),              // Parameter count
            argTypes.str(),                                // Argument types
            visibilityToString(func->visibility)};         // Visibility
        entry.type = TableType::FUNCTION_TABLE;

        return entry;
    }

    TableEntry SymbolTableDB::symbolToVariableEntry(const Symbol *symbol) const
    {
        TableEntry entry;
        if (!symbol || !symbol->variable)
            return entry;

        const VariableSymbol *var = symbol->variable;

        entry.columns = {
            symbolTypeToString(symbol->symbolType), // Symbol type
            var->name,                              // Name
            DataTypeToStringUnformatted(var->type), // Type
            var->scopeId,                           // ID
            std::to_string(0),                      // Scope depth
            visibilityToString(VISIBILITY_PUBLIC)}; // Visibility
        entry.type = TableType::VARIABLE_TABLE;

        return entry;
    }

    TableEntry SymbolTableDB::symbolToTypeEntry(const Symbol *symbol) const
    {
        TableEntry entry;
        if (!symbol || !symbol->type)
            return entry;

        const TypeSymbol *type = symbol->type;

        // Build parameter types string
        std::stringstream paramTypes;
        for (size_t i = 0; i < 0; i++)
        {
            if (i > 0)
                paramTypes << ",";
            paramTypes << "param_type_placeholder"; // Replace with actual parameter type
        }

        entry.columns = {
            symbolTypeToString(symbol->symbolType),  // Symbol type
            type->name,                              // Name
            DataTypeToStringUnformatted(type->type), // Type
            type->scopeId,                           // ID
            std::to_string(0),                       // Scope depth
            std::to_string(0),                       // Property count
            paramTypes.str(),                        // Parameter types
            visibilityToString(VISIBILITY_PUBLIC)};  // Visibility
        entry.type = TableType::TYPE_TABLE;

        return entry;
    }

    std::string SymbolTableDB::symbolTypeToString(TypeOfSymbol type) const
    {
        switch (type)
        {
        case FUNCTION_SYMBOL:
            return "FUNCTION";
        case VARIABLE_SYMBOL:
            return "VARIABLE";
        case TYPE_SYMBOL:
            return "TYPE";
        default:
            return "UNKNOWN";
        }
    }

    std::string SymbolTableDB::visibilityToString(CryoVisibilityType visibility) const
    {
        switch (visibility)
        {
        case VISIBILITY_PUBLIC:
            return "public";
        case VISIBILITY_PRIVATE:
            return "private";
        case VISIBILITY_PROTECTED:
            return "protected";
        default:
            return "unknown";
        }
    }

    bool SymbolTableDB::insertRow(TableType type, const TableEntry &entry)
    {
        if (!validateEntry(type, entry))
            return false;

        FILE *file = openTable(type, "a");
        if (!file)
            return false;

        bool success = writeEntry(file, entry);
        closeTable(file);
        return success;
    }

    bool SymbolTableDB::updateRow(TableType type, size_t rowId, const TableEntry &entry)
    {
        if (!validateEntry(type, entry))
            return false;

        FILE *file = openTable(type, "r+");
        if (!file)
            return false;

        if (!seekToRow(file, rowId))
        {
            closeTable(file);
            return false;
        }

        bool success = writeEntry(file, entry);
        closeTable(file);
        return success;
    }

    std::vector<TableEntry> SymbolTableDB::queryTable(TableType type) const
    {
        std::vector<TableEntry> results;
        FILE *file = openTable(type, "r");
        if (!file)
        {
            std::cerr << "Failed to open table file" << std::endl;
            return results;
        }

        char buffer[READ_BUFFER_SIZE];

        // Skip the header (3 lines: top border, column names, separator)
        for (int i = 0; i < 3; i++)
        {
            if (fgets(buffer, sizeof(buffer), file) == nullptr)
            {
                std::cerr << "Failed to read header line " << i << std::endl;
                closeTable(file);
                return results;
            }
        }

        // Read entries
        while (!feof(file))
        {
            TableEntry entry = readEntry(file);
            if (!entry.columns.empty())
            {
                entry.type = type; // Set the table type
                results.push_back(entry);
            }
        }

        closeTable(file);

        std::cout << "Read " << results.size() << " entries from table" << std::endl;
        return results;
    }

    TableEntry SymbolTableDB::queryRow(TableType type, size_t rowId)
    {
        TableEntry entry;
        FILE *file = openTable(type, "r");
        if (!file)
            return entry;

        if (seekToRow(file, rowId))
        {
            entry = readEntry(file);
        }

        closeTable(file);
        return entry;
    }

    bool SymbolTableDB::validateEntry(TableType type, const TableEntry &entry) const
    {
        auto formats = getColumnFormats(type);
        if (entry.columns.size() != formats.size())
        {
            return false;
        }

        // Validate each column's content length
        for (size_t i = 0; i < entry.columns.size(); ++i)
        {
            if (entry.columns[i].length() > formats[i].width - 2)
            {
                // Either truncate here or return false depending on your needs
            }
        }
        return true;
    }

    bool SymbolTableDB::dropTable(TableType type)
    {
        FILE *file = openTable(type, "a");
        if (file)
        {
            writeTableFooter(file);
            closeTable(file);
        }

        return std::filesystem::remove(getTablePath(type));
    }

    bool SymbolTableDB::deserializeToSymbolTable(SymbolTable *table)
    {
        if (!table)
            return false;

        // Clear existing symbols
        table->count = 0;
        table->symbols = nullptr;

        // Query each table and add symbols to the symbol table
        std::vector<TableEntry> functionEntries = queryTable(TableType::FUNCTION_TABLE);
        for (const TableEntry &entry : functionEntries)
        {
            Symbol *symbol = entryToSymbol(entry);
            if (symbol)
            {
                addSingleSymbolToTable(symbol, table);
            }
        }

        std::vector<TableEntry> variableEntries = queryTable(TableType::VARIABLE_TABLE);
        for (const TableEntry &entry : variableEntries)
        {
            Symbol *symbol = entryToSymbol(entry);
            if (symbol)
            {
                addSingleSymbolToTable(symbol, table);
            }
        }

        std::vector<TableEntry> typeEntries = queryTable(TableType::TYPE_TABLE);
        for (const TableEntry &entry : typeEntries)
        {
            Symbol *symbol = entryToSymbol(entry);
            if (symbol)
            {
                addSingleSymbolToTable(symbol, table);
            }
        }

        return true;
    }

    Symbol *SymbolTableDB::entryToSymbol(const TableEntry &entry) const
    {
        if (entry.columns.empty())
            return nullptr;

        TypeOfSymbol symbolType = stringToSymbolType(entry.columns[0]);
        switch (symbolType)
        {
        case FUNCTION_SYMBOL:
            return functionEntryToSymbol(entry);
        case VARIABLE_SYMBOL:
            return variableEntryToSymbol(entry);
        case TYPE_SYMBOL:
            return typeEntryToSymbol(entry);
        default:
            return nullptr;
        }
    }

    Symbol *SymbolTableDB::functionEntryToSymbol(const TableEntry &entry) const
    {
        Symbol *symbol = new Symbol();
        symbol->symbolType = FUNCTION_SYMBOL;
        symbol->function = new FunctionSymbol();

        FunctionSymbol *func = symbol->function;
        func->name = entry.columns[1].c_str();
        // func->returnType = DataTypeFromString(entry.columns[2].c_str()); // Implement this
        func->paramCount = std::stoi(entry.columns[5]);
        // func->paramTypes = new DataType *[func->paramCount]; // Implement this
        func->visibility = stringToVisibilityType(entry.columns[7].c_str());
        func->functionScopeId = entry.columns[3].c_str();

        return symbol;
    }
    Symbol *SymbolTableDB::variableEntryToSymbol(const TableEntry &entry) const
    {
        Symbol *symbol = new Symbol();
        symbol->symbolType = VARIABLE_SYMBOL;
        symbol->variable = new VariableSymbol();

        VariableSymbol *var = symbol->variable;
        var->name = entry.columns[1].c_str();
        // var->type = DataTypeFromString(entry.columns[2].c_str()); // Implement this
        var->scopeId = entry.columns[3].c_str();

        return symbol;
    }
    Symbol *SymbolTableDB::typeEntryToSymbol(const TableEntry &entry) const
    {
        Symbol *symbol = new Symbol();
        symbol->symbolType = TYPE_SYMBOL;
        symbol->type = new TypeSymbol();

        TypeSymbol *type = symbol->type;
        type->name = entry.columns[1].c_str();
        // type->type = DataTypeFromString(entry.columns[2].c_str()); // Implement this
        type->scopeId = entry.columns[3].c_str();

        return symbol;
    }

    TypeOfSymbol SymbolTableDB::stringToSymbolType(const std::string &type) const
    {
        if (type == "FUNCTION")
            return FUNCTION_SYMBOL;
        if (type == "VARIABLE")
            return VARIABLE_SYMBOL;
        if (type == "TYPE")
            return TYPE_SYMBOL;
        return UNKNOWN_SYMBOL;
    }

    void SymbolTableDB::addSingleSymbolToTable(Symbol *symbol, SymbolTable *table)
    {
        if (!symbol || !table)
            return;

        if (!table->symbols)
        {
            table->symbols = (Symbol **)malloc(sizeof(Symbol *) * MAX_SYMBOLS);
        }

        if (table->count < MAX_SYMBOLS)
        {
            table->symbols[table->count++] = symbol;
        }
    }

    size_t SymbolTableDB::getColumnCount(TableType type) const
    {
        switch (type)
        {
        case TableType::FUNCTION_TABLE:
            return FUNCTION_TABLE_COLS;
        case TableType::VARIABLE_TABLE:
            return VARIABLE_TABLE_COLS;
        case TableType::TYPE_TABLE:
            return TYPE_TABLE_COLS;
        }
        return 0;
    }

    std::vector<TableEntry> SymbolTableDB::queryByColumn(TableType type, size_t columnIndex, const std::string &value)
    {
        std::vector<TableEntry> results;
        FILE *file = openTable(type, "r");
        if (!file)
            return results;

        std::string line;
        size_t currentRow = 0;
        while (!feof(file))
        {
            TableEntry entry = readEntry(file);
            if (!entry.columns.empty() && entry.columns.size() > columnIndex && entry.columns[columnIndex] == value)
            {
                results.push_back(entry);
            }
        }

        closeTable(file);
        return results;
    }

    std::string SymbolTableDB::getTablePath(TableType type) const
    {
        std::string filename;
        switch (type)
        {
        case TableType::FUNCTION_TABLE:
            filename = "functions.db";
            break;
        case TableType::VARIABLE_TABLE:
            filename = "variables.db";
            break;
        case TableType::TYPE_TABLE:
            filename = "types.db";
            break;
        }
        return dirs.DBdir + "/" + filename;
    }

    void SymbolTableDB::serializeEntry(const TableEntry &entry, std::string &output) const
    {
        std::vector<Cryo::SymbolTableDB::ColumnFormat> formats = getColumnFormats(entry.type);
        const std::vector<size_t> columnWidths = getColumnWidths(formats);
        std::stringstream ss;

        // Add entry data
        ss << _VERTICAL;
        for (size_t i = 0; i < entry.columns.size(); ++i)
        {
            std::string value = entry.columns[i];
            if (value.length() > columnWidths[i] - 2)
            {
                value = value.substr(0, columnWidths[i] - 5) + "...";
            }
            ss << ' ' << std::left << std::setw(columnWidths[i] - 1) << value << _VERTICAL;
        }
        ss << "\n";

        // Add separator after each entry
        ss << _T_RIGHT;
        for (size_t i = 0; i < columnWidths.size(); ++i)
        {
            ss << std::string(columnWidths[i], _HORIZONTAL[0]);
            ss << (i < columnWidths.size() - 1 ? _CROSS : _T_LEFT);
        }
        ss << "\n";

        output = ss.str();
    }

    const std::vector<size_t> SymbolTableDB::getColumnWidths(const std::vector<ColumnFormat> &formats) const
    {
        std::vector<size_t> widths;
        for (const auto &col : formats)
        {
            widths.push_back(col.width);
        }
        return widths;
    }

    TableEntry SymbolTableDB::deserializeEntry(const std::string &input) const
    {
        TableEntry entry;
        std::stringstream ss(input);
        std::string column;

        while (std::getline(ss, column, '|'))
        {
            entry.columns.push_back(column);
        }

        return entry;
    }

    FILE *SymbolTableDB::openTable(TableType type, const char *mode) const
    {
        return fopen(getTablePath(type).c_str(), mode);
    }

    void SymbolTableDB::closeTable(FILE *file) const
    {
        if (file)
            fclose(file);
    }

    bool SymbolTableDB::writeEntry(FILE *file, const TableEntry &entry)
    {
        std::string serialized;
        serializeEntry(entry, serialized);
        return fwrite(serialized.c_str(), 1, serialized.length(), file) == serialized.length();
    }

    TableEntry SymbolTableDB::readEntry(FILE *file) const
    {
        TableEntry entry;
        char buffer[READ_BUFFER_SIZE];

        // Read a line
        if (!fgets(buffer, sizeof(buffer), file))
        {
            return entry;
        }

        std::string line(buffer);

        // Skip separator lines
        if (line.empty() || line[0] == '+' || line.find_first_not_of("-+| \n\r\t") == std::string::npos)
        {
            return entry;
        }

        // Split the line by '|' character
        std::vector<std::string> parts;
        std::string part;
        std::stringstream ss(line);

        // Skip the first empty part (before first '|')
        if (!std::getline(ss, part, '|'))
        {
            return entry;
        }

        // Read the actual columns
        while (std::getline(ss, part, '|'))
        {
            // Trim whitespace
            auto start = part.find_first_not_of(" \t\r\n");
            auto end = part.find_last_not_of(" \t\r\n");

            if (start != std::string::npos && end != std::string::npos)
            {
                entry.columns.push_back(part.substr(start, end - start + 1));
            }
        }

        // Skip the separator line after the entry
        fgets(buffer, sizeof(buffer), file);

        return entry;
    }

    bool SymbolTableDB::seekToRow(FILE *file, size_t rowId)
    {
        fseek(file, 0, SEEK_SET);
        for (size_t i = 0; i < rowId; ++i)
        {
            char buffer[1024];
            if (!fgets(buffer, sizeof(buffer), file))
            {
                return false;
            }
        }
        return true;
    }

    bool SymbolTableDB::deleteRow(TableType type, size_t rowId)
    {
        // Open the table in read mode
        FILE *file = openTable(type, "r");
        if (!file)
            return false;

        // Open a temporary file for writing
        std::string tempPath = getTablePath(type) + ".tmp";
        FILE *tempFile = fopen(tempPath.c_str(), "w+");
        if (!tempFile)
        {
            closeTable(file);
            return false;
        }

        // Copy all rows except the one to delete
        size_t currentRow = 0;
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file))
        {
            if (currentRow != rowId)
            {
                fwrite(buffer, 1, strlen(buffer), tempFile);
            }
            currentRow++;
        }

        // Close both files
        closeTable(file);
        closeTable(tempFile);

        // Replace the original file with the temporary file
        std::filesystem::remove(getTablePath(type));
        std::filesystem::rename(tempPath, getTablePath(type));

        return true;
    }

    void SymbolTableDB::writeTableHeader(FILE *file, TableType type) const
    {
        std::string header = getTableHeader(type);
        fwrite(header.c_str(), 1, header.length(), file);
    }

    std::string SymbolTableDB::getTableHeader(TableType type) const
    {
        std::vector<ColumnFormat> formats = getColumnFormats(type);
        std::stringstream ss;

        // Top border
        ss << _TOP_LEFT;
        for (size_t i = 0; i < formats.size(); ++i)
        {
            ss << std::string(formats[i].width, _HORIZONTAL[0]);
            ss << (i < formats.size() - 1 ? _T_DOWN : _TOP_RIGHT);
        }
        ss << "\n";

        // Column headers
        ss << _VERTICAL;
        for (const auto &col : formats)
        {
            std::string name = col.name;
            if (name.length() > col.width - 2)
            {
                name = name.substr(0, col.width - 5) + "...";
            }
            ss << ' ' << std::left << std::setw(col.width - 1) << name << _VERTICAL;
        }
        ss << "\n";

        // Separator line
        ss << _T_RIGHT;
        for (size_t i = 0; i < formats.size(); ++i)
        {
            ss << std::string(formats[i].width, _HORIZONTAL[0]);
            ss << (i < formats.size() - 1 ? _CROSS : _T_LEFT);
        }
        ss << "\n";

        return ss.str();
    }

    void SymbolTableDB::writeTableFooter(FILE *file) const
    {
        auto formats = getColumnFormats(TableType::FUNCTION_TABLE);
        std::stringstream ss;

        // Bottom border
        ss << _BOTTOM_LEFT;
        for (size_t i = 0; i < formats.size(); ++i)
        {
            ss << std::string(formats[i].width, _HORIZONTAL[0]);
            ss << (i < formats.size() - 1 ? T_UP : _BOTTOM_RIGHT);
        }
        ss << "\n";

        std::string footer = ss.str();
        fwrite(footer.c_str(), 1, footer.length(), file);
    }

    std::vector<SymbolTableDB::ColumnFormat> SymbolTableDB::getColumnFormats(TableType type) const
    {
        std::vector<ColumnFormat> formats;

        switch (type)
        {
        case TableType::FUNCTION_TABLE:
            formats = {
                {"TYPE", 10},
                {"NAME", 20},
                {"RETURN_TYPE", 25},
                {"ID", 20},
                {"DEPTH", 6},
                {"ARGS", 6},
                {"ARG_TYPES", 20},
                {"VISIBLE", 10}};
            break;

        case TableType::VARIABLE_TABLE:
            formats = {
                {"TYPE", 10},
                {"NAME", 20},
                {"VAR_TYPE", 25},
                {"ID", 20},
                {"DEPTH", 10},
                {"VISIBLE", 25}};

            break;

        case TableType::TYPE_TABLE:
            formats = {
                {"TYPE", 10},
                {"NAME", 20},
                {"BASE_TYPE", 25},
                {"ID", 20},
                {"DEPTH", 6},
                {"PARAMS", 6},
                {"PARAM_TYPES", 20},
                {"VISIBLE", 10}};
            break;
        }

        return formats;
    }

    bool SymbolTableDB::createScopedDB() const
    {
        // Create chunks.db file
        std::string chunkPath = getChunkPath();
        FILE *chunkFile = fopen(chunkPath.c_str(), "w+");
        if (!chunkFile)
        {
            std::cerr << "Failed to create chunks.db" << std::endl;
            return false;
        }

        // Write header
        writeChunkHeader(chunkFile);

        // Group entries by ID
        std::vector<ChunkEntry> chunks = groupEntriesByID();

        // Write each chunk
        for (const auto &chunk : chunks)
        {
            if (validateChunkEntry(chunk))
            {
                writeChunkEntry(chunkFile, chunk);
            }
        }

        // Write footer and close
        writeTableFooter(chunkFile);
        closeTable(chunkFile);
        return true;
    }

    std::string SymbolTableDB::getChunkPath() const
    {
        return dirs.DBdir + "/chunks.db";
    }

    void SymbolTableDB::writeChunkHeader(FILE *file) const
    {
        auto formats = getColumnFormats(TableType::FUNCTION_TABLE);
        size_t totalWidth = std::accumulate(formats.begin(), formats.end(), 0,
                                            [](size_t sum, const ColumnFormat &fmt)
                                            { return sum + fmt.width; });
        totalWidth += formats.size() + 1; // Add width for separators and edges

        std::stringstream ss;

        // Top border
        ss << _TOP_LEFT << std::string(totalWidth - 2, _HORIZONTAL[0]) << _TOP_RIGHT << "\n";

        // Title
        ss << _VERTICAL << std::setw((totalWidth + 10) / 2) << "SCOPE TABLE"
           << std::setw((totalWidth - 10) / 2) << _VERTICAL << "\n";

        // Bottom border
        ss << _T_RIGHT << std::string(totalWidth - 2, _HORIZONTAL[0]) << _T_LEFT << "\n";

        std::string headerStr = ss.str();
        fwrite(headerStr.c_str(), 1, headerStr.length(), file);
    }

    void SymbolTableDB::writeChunkEntry(FILE *file, const ChunkEntry &chunk) const
    {
        auto formats = getColumnFormats(TableType::FUNCTION_TABLE);
        size_t totalWidth = std::accumulate(formats.begin(), formats.end(), 0,
                                            [](size_t sum, const ColumnFormat &fmt)
                                            { return sum + fmt.width; });
        totalWidth += formats.size() + 1;

        std::stringstream ss;

        // Add initial newline before new chunk starts (except for first chunk)
        static bool firstChunk = true;
        if (!firstChunk)
        {
            ss << "\n";
        }
        firstChunk = false;

        // Write scope header
        ss << _VERTICAL << " SCOPE: " << std::left << std::setw(20) << chunk.functionEntry.columns[3]
           << std::right << std::setw(totalWidth - 29) << _VERTICAL << "\n";

        // Rest of the function remains the same until the end separator
        ss << _T_RIGHT;
        for (size_t i = 0; i < formats.size(); ++i)
        {
            ss << std::string(formats[i].width, _HORIZONTAL[0]);
            ss << (i < formats.size() - 1 ? _CROSS : _T_LEFT);
        }
        ss << "\n";

        std::string functionStr;
        serializeEntry(chunk.functionEntry, functionStr);
        ss << functionStr;

        if (!chunk.variables.empty())
        {
            ss << _VERTICAL << " Variables:" << std::right << std::setw(totalWidth - 12) << _VERTICAL << "\n";

            ss << _T_RIGHT;
            for (size_t i = 0; i < formats.size(); ++i)
            {
                ss << std::string(formats[i].width, _HORIZONTAL[0]);
                ss << (i < formats.size() - 1 ? _CROSS : _T_LEFT);
            }
            ss << "\n";

            for (const auto &var : chunk.variables)
            {
                std::string varStr;
                serializeEntry(var, varStr);
                ss << varStr;
            }
        }

        ss << "\n"; // Only one newline at the end

        // Write end separator
        ss << _T_RIGHT;
        for (size_t i = 0; i < formats.size(); ++i)
        {
            ss << std::string(formats[i].width, _HORIZONTAL[0]);
            ss << (i < formats.size() - 1 ? _CROSS : _T_LEFT);
        }

        std::string outputStr = ss.str();
        fwrite(outputStr.c_str(), 1, outputStr.length(), file);
    }

    std::vector<SymbolTableDB::ChunkEntry> SymbolTableDB::groupEntriesByID() const
    {
        std::vector<ChunkEntry> chunks;
        try
        {
            std::map<std::string, ChunkEntry> idToChunk;

            // Get all functions
            std::vector<TableEntry> functions = queryTable(TableType::FUNCTION_TABLE);
            std::cout << "Processing " << functions.size() << " functions" << std::endl;

            // Get all variables
            std::vector<TableEntry> variables = queryTable(TableType::VARIABLE_TABLE);
            std::cout << "Processing " << variables.size() << " variables" << std::endl;

            // Group functions by ID with safety checks
            for (const auto &func : functions)
            {
                if (func.columns.size() <= 3)
                {
                    std::cerr << "Invalid function entry (column count: " << func.columns.size() << ")" << std::endl;
                    continue;
                }

                std::string id;
                try
                {
                    id = std::string(func.columns[3]);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error copying function ID: " << e.what() << std::endl;
                    continue;
                }

                ChunkEntry chunk;
                chunk.functionEntry = func;
                chunk.variables.reserve(10);
                idToChunk.emplace(id, std::move(chunk));
            }

            // Add variables to chunks
            for (const auto &var : variables)
            {
                if (var.columns.size() <= 3)
                    continue;
                std::string id = var.columns[3];

                auto it = idToChunk.find(id);
                if (it != idToChunk.end())
                {
                    it->second.variables.push_back(var);
                }
            }

            chunks.reserve(idToChunk.size());
            for (auto &pair : idToChunk)
            {
                chunks.push_back(std::move(pair.second));
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Fatal error in groupEntriesByID: " << e.what() << std::endl;
        }

        return chunks;
    }

    bool SymbolTableDB::validateChunkEntry(const ChunkEntry &chunk) const
    {
        // Validate function entry
        if (!validateEntry(TableType::FUNCTION_TABLE, chunk.functionEntry))
        {
            return false;
        }

        // Validate all variable entries
        for (const auto &var : chunk.variables)
        {
            if (!validateEntry(TableType::VARIABLE_TABLE, var))
            {
                return false;
            }
        }

        return true;
    }

} // namespace Cryo
