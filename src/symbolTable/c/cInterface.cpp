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
#include "symbolTable/globalSymtable.hpp"

// =======================================================
// C API Functions

extern "C"
{
    CryoGlobalSymbolTable *CryoGlobalSymbolTable_Create()
    {
        try
        {
            auto symTable = new Cryo::GlobalSymbolTable();
            return reinterpret_cast<CryoGlobalSymbolTable *>(symTable);
        }
        catch (...)
        {
            fprintf(stderr, "Failed to create GlobalSymbolTable\n");
            return nullptr;
        }
    }

    void CryoGlobalSymbolTable_PrintGlobalTable(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->printGlobalTable(
                reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable));
        }
    }

    // --------------------------------------------------------------
    // Class State Functions

    bool CryoGlobalSymbolTable_GetIsPrimaryTable(CryoGlobalSymbolTable *symTable)
    {
        return reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->getIsPrimaryTable();
    }

    bool CryoGlobalSymbolTable_GetIsDependencyTable(CryoGlobalSymbolTable *symTable)
    {
        return reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->getIsDependencyTable();
    }

    void CryoGlobalSymbolTable_SetPrimaryTableStatus(CryoGlobalSymbolTable *symTable, bool isPrimary)
    {
        reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->setIsPrimaryTable(isPrimary);
    }

    void CryoGlobalSymbolTable_SetDependencyTableStatus(CryoGlobalSymbolTable *symTable, bool isDependency)
    {
        reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->setIsDependencyTable(isDependency);
    }

    void CryoGlobalSymbolTable_TableFinished(CryoGlobalSymbolTable *symTable)
    {
        reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->tableFinished();
    }

    void CryoGlobalSymbolTable_InitNamespace(CryoGlobalSymbolTable *symTable, const char *namespaceName)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->initNamespace(namespaceName);
        }
    }

    // --------------------------------------------------------------
    // Symbol Table Functions

    void CryoGlobalSymbolTable_InitDependencyTable(CryoGlobalSymbolTable *symTable, const char *namespaceName)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->initDependencyTable(namespaceName);
        }
    }

    void CryoGlobalSymbolTable_CreatePrimaryTable(CryoGlobalSymbolTable *symTable, const char *namespaceName)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->createPrimaryTable(namespaceName);
        }
    }

    void CryoGlobalSymbolTable_AddNodeToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->addNodeToTable(node);
        }
    }

    void CryoGlobalSymbolTable_AddVariableToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node, const char *scopeID)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->addVariableToSymbolTable(node, scopeID);
        }
    }

    void CryoGlobalSymbolTable_AddParamToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node, const char *functionScopeID)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->addParamToSymbolTable(node, functionScopeID);
        }
    }

    SymbolTable *CryoGlobalSymbolTable_GetCurrentSymbolTable(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            return reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->getCurrentSymbolTable();
        }
        return nullptr;
    }

    // --------------------------------------------------------------
    // Scope Functions

    void CryoGlobalSymbolTable_EnterScope(CryoGlobalSymbolTable *symTable, const char *name)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->enterScope(name);
        }
    }

    void CryoGlobalSymbolTable_ExitScope(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->exitScope();
        }
    }

    const char *CryoGlobalSymbolTable_GetScopeID(CryoGlobalSymbolTable *symTable, const char *name)
    {
        if (symTable)
        {
            return reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->getScopeID(name);
        }
        return nullptr;
    }

    // --------------------------------------------------------------
    // Symbol Resolution Functions

    VariableSymbol *CryoGlobalSymbolTable_GetFrontendVariableSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID)
    {
        if (symTable)
        {
            return reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->getFrontendVariableSymbol(name, scopeID);
        }
        return nullptr;
    }

    // --------------------------------------------------------------
    // Debug Functions

    void CryoGlobalSymbolTable_MergeDBChunks(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->mergeDBChunks();
        }
    }

    // --------------------------------------------------------------
    // Declaration Functions (Classes)

    void CryoGlobalSymbolTable_InitClassDeclaration(CryoGlobalSymbolTable *symTable, const char *className)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->initClassDeclaration(className);
        }
    }

    void CryoGlobalSymbolTable_AddPropertyToClass(CryoGlobalSymbolTable *symTable, const char *className, ASTNode *property)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->addPropertyToClass(className, property);
        }
    }

    void CryoGlobalSymbolTable_AddMethodToClass(CryoGlobalSymbolTable *symTable, const char *className, ASTNode *method)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->addMethodToClass(className, method);
        }
    }

    void CryoGlobalSymbolTable_CompleteClassDeclaration(CryoGlobalSymbolTable *symTable, ASTNode *classNode, const char *className)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->completeClassDeclaration(classNode, className);
        }
    }

    // --------------------------------------------------------------
    // Declaration Functions (Structs)

    void CryoGlobalSymbolTable_InitStructDeclaration(CryoGlobalSymbolTable *symTable, const char *structName, const char *parentNameID)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->initStructDeclaration(structName, parentNameID);
        }
    }

    void CryoGlobalSymbolTable_AddPropertyToStruct(CryoGlobalSymbolTable *symTable, const char *structName, ASTNode *property)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->addPropertyToStruct(structName, property);
        }
    }

    void CryoGlobalSymbolTable_AddMethodToStruct(CryoGlobalSymbolTable *symTable, const char *className, ASTNode *method)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->addMethodToStruct(className, method);
        }
    }

    void CryoGlobalSymbolTable_CompleteStructDeclaration(CryoGlobalSymbolTable *symTable, ASTNode *structNode, const char *structName)
    {
        if (symTable)
        {
            reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->completeStructDeclaration(structNode, structName);
        }
    }

    // --------------------------------------------------------------
    // Symbol Resolution Functions

    Symbol *CryoGlobalSymbolTable_GetFrontendSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID, TypeOfSymbol symbolType)
    {
        if (symTable)
        {
            return reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->getFrontendSymbol(name, scopeID, symbolType);
        }
        return nullptr;
    }

    FunctionSymbol *CryoGlobalSymbolTable_ResolveScopedFunctionSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID)
    {
        if (symTable)
        {
            return reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->resolveScopedFunctionCall(scopeID, name);
        }
        return nullptr;
    }

    Symbol *CryoGlobalSymbolTable_FindSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID)
    {
        if (symTable)
        {
            return reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->findSymbol(name, scopeID);
        }
        return nullptr;
    }

    DataType *CryoGlobalSymbolTable_GetDataTypeFromSymbol(CryoGlobalSymbolTable *symTable, Symbol *symbol)
    {
        if (symTable)
        {
            return reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->getDataTypeFromSymbol(symbol);
        }
        return nullptr;
    }

    ASTNode *CryoGlobalSymbolTable_GetASTNodeFromSymbol(CryoGlobalSymbolTable *symTable, Symbol *symbol)
    {
        if (symTable)
        {
            return reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->getASTNodeFromSymbol(symbol);
        }
        return nullptr;
    }

    Symbol *CryoGlobalSymbolTable_FindMethodSymbol(CryoGlobalSymbolTable *symTable, const char *methodName, const char *className, TypeofDataType typeOfNode)
    {
        if (symTable)
        {
            return reinterpret_cast<Cryo::GlobalSymbolTable *>(symTable)->findMethodSymbol(methodName, className, typeOfNode);
        }
        return nullptr;
    }

} // C API ----------------------------------------------------------
