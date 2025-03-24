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
#ifndef AST_H
#define AST_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#include "frontend/tokens.h"
#include "frontend/lexer.h"
#include "tools/arena/arena.h"
#include "tools/utils/c_logger.h"
#include "tools/macros/consoleColors.h"
#include "dataTypes/dataTypes.h"
#include "common/common.h"
#include "tools/utils/cWrappers.h"

typedef struct DataType_t DataType;
typedef struct GenericType GenericType;
typedef struct ParsingContext ParsingContext;
typedef struct CompilerState CompilerState;
typedef struct Lexer Lexer;

typedef struct ASTNode ASTNode;
typedef struct CompilerSettings CompilerSettings;
typedef struct CryoGlobalSymbolTable_t *CryoGlobalSymbolTable;

#define INITIAL_CAPACITY 8
#define PROGRAM_CAPACITY 512
#define BLOCK_CAPACITY 512
#define FUNCTION_BLOCK_CAPACITY 512
#define PARAM_CAPACITY 16
#define ARG_CAPACITY 16
#define GENERIC_CAPACITY 16
#define IMPORT_CAPACITY 16
#define ARRAY_CAPACITY 128
#define PROPERTY_CAPACITY 32
#define METHOD_CAPACITY 64

/// ### The position struct represents the line and column number of a token in the source code.
typedef struct Position
{
    int line;
    int column;
} Position;

typedef struct CryoModule
{
    ASTNode **astTable;
} CryoModule;

typedef struct CryoNamespace
{
    char *name;
    const char *parentName;
    bool hasParent;
    Position position;
} CryoNamespace;

typedef struct CryoProgram
{
    struct ASTNode **statements;
    size_t statementCount;
    size_t statementCapacity;
} CryoProgram;

typedef struct CryoMetaData
{
    enum CryoNodeType type; // Node Type
    int line;               // Line number for error reporting
    int column;             // Column number for error reporting
    const char *lexeme;     // Lexeme

    char *moduleName; // Current Module
    Position position;
} CryoMetaData;

typedef struct CryoImportNode
{
    char *moduleName;
    char *subModuleName;
    bool isStdModule;
} CryoImportNode;

typedef struct CryoScope
{
    int scopeLevel;
    char *scopeName;
    CryoNodeType scopeType;
} CryoScope;

typedef struct CryoBlockNode
{
    struct ASTNode **statements;
    int statementCount;
    int statementCapacity;
} CryoBlockNode;

typedef struct CryoFunctionBlock
{
    struct ASTNode *function;
    struct ASTNode **statements;
    int statementCount;
    int statementCapacity;
} CryoFunctionBlock;

typedef struct ExternNode
{
    struct ASTNode *externNode;
} ExternNode;

typedef struct ExternFunctionNode
{
    char *name;
    struct ASTNode **params;
    int paramCount;
    int paramCapacity;
    DataType *type;

    const char *parentScopeID;
} ExternFunctionNode;

typedef struct FunctionDeclNode
{
    CryoNodeType nodeType;
    CryoVisibilityType visibility;
    DataType *functionType;
    DataType *type;

    const char *name;
    struct ASTNode **params;
    int paramCount;
    int paramCapacity;
    DataType **paramTypes;
    struct ASTNode *body;

    const char *parentScopeID;
    const char *functionScopeID;

    bool isVariadic;
    GenericType **genericParams;
    int genericParamCount;
} FunctionDeclNode;

typedef struct FunctionCallNode
{
    char *name;
    struct ASTNode **args;
    int argCount;
    int argCapacity;
    DataType *returnType;
    bool isVariadic;
} FunctionCallNode;

typedef struct LiteralNode
{
    DataType *type;
    int length;
    union
    {
        int intValue;
        float floatValue;
        const char *stringValue;
        int booleanValue;
    } value;
} LiteralNode;

typedef struct IfStatementNode
{
    struct ASTNode *condition;
    struct ASTNode *thenBranch;
    struct ASTNode *elseBranch;
} IfStatementNode;

typedef struct ForStatementNode
{
    // `${initializer};`
    struct ASTNode *initializer;
    // `${condition};`
    struct ASTNode *condition;
    // `${increment};`
    struct ASTNode *increment;
    CryoOperatorType incrementOp;
    // { body }
    struct ASTNode *body;
} ForStatementNode;

typedef struct WhileStatementNode
{
    struct ASTNode *condition;
    struct ASTNode *body;
} WhileStatementNode;

typedef struct VariableNameNode
{
    DataType *type;
    bool isRef; // Remove Later
    char *varName;
    const char *scopeID;

    // Unary operators
    bool hasUnaryOp;
    ASTNode *unaryOp;
} VariableNameNode;

typedef struct CryoExpressionNode
{
    CryoNodeType nodeType;
    DataType *type;
    union
    {
        // For Variable References
        VariableNameNode *varNameNode;
        LiteralNode *literalNode;
        // Unsure what else to put here but I'll figure it out later
    } data;
} CryoExpressionNode;

typedef struct CryoVariableNode
{
    DataType *type;
    struct VariableNameNode *varNameNode;
    char *name;
    bool isGlobal;
    bool isLocal;
    bool isReference;
    bool isMutable;
    bool isIterator;
    bool isNewInstance;
    // Unary operators
    bool hasUnaryOp;
    ASTNode *unaryOp;
    // This is the data attached to the variable
    struct ASTNode *initializer;
    // Optional index expression for array handling
    bool hasIndexExpr;
    struct IndexExprNode *indexExpr;
} CryoVariableNode;

typedef struct CryoParameterNode
{
    DataType *type;
    const char *name;
    char *functionName;

    struct ASTNode *defaultValue;
    bool hasDefaultValue;

    bool isMutable;

    bool isVariadic;
    DataType *variadicElementType; // For typed varargs like T[]
} CryoParameterNode;

typedef struct ParamNode
{
    enum CryoNodeType nodeType;
    CryoVariableNode **params;
    int paramCount;
    int paramCapacity;
    char *funcRefName;
} ParamNode;

typedef struct ArgNode
{
    enum CryoNodeType nodeType;
    ASTNode **args;
    int argCount;
    int argCapacity;
    char *funcRefName;
} ArgNode;

typedef struct CryoReturnNode
{
    struct ASTNode *returnValue;
    struct ASTNode *expression;
    DataType *type;
} CryoReturnNode;

typedef struct CryoBinaryOpNode
{
    struct ASTNode *left;
    struct ASTNode *right;
    CryoOperatorType op;
} CryoBinaryOpNode;

typedef struct CryoUnaryOpNode
{
    CryoTokenType op;
    DataType *resultType;
    struct ASTNode *operand;
    struct ASTNode *expression;
} CryoUnaryOpNode;

typedef struct CryoArrayNode
{
    DataType *type;
    DataType **elementTypes;
    struct ASTNode **elements;
    int elementCount;
    int elementCapacity;
} CryoArrayNode;

typedef struct IndexExprNode
{
    // Name of the array reference
    char *name;
    // Should be the array node itself
    struct ASTNode *array;
    // Should be a literal expression node with an int value
    struct ASTNode *index;
} IndexExprNode;

typedef struct VariableReassignmentNode
{
    char *existingVarName;
    ASTNode *existingVarNode;
    DataType *existingVarType;
    DataType *type;
    ASTNode *newVarNode;
} VariableReassignmentNode;

typedef struct PropertyNode
{
    const char *name;
    ASTNode *value;
    DataType *type;
    bool defaultProperty;
    const char *parentName;
    CryoNodeType parentNodeType;
} PropertyNode;

typedef struct MethodNode
{
    DataType *functionType;
    DataType *type;
    char *name;
    ASTNode **params;
    int paramCount;
    int paramCapacity;
    DataType **paramTypes;
    ASTNode *body;
    CryoVisibilityType visibility;
    bool isStatic;
    const char *parentName;
} MethodNode;

typedef struct MethodCallNode
{
    DataType *returnType;
    DataType *instanceType;
    ASTNode *accessorObj;
    const char *instanceName;
    const char *name;
    ASTNode **args;
    int argCount;
    int argCapacity;
    bool isStatic;
} MethodCallNode;

typedef struct StructNode
{
    const char *name;
    ASTNode **properties;
    ASTNode **methods;
    ASTNode *constructor;
    ASTNode **ctorArgs;

    int ctorArgCount;
    int ctorArgCapacity;

    int propertyCount;
    int propertyCapacity;

    int methodCount;
    int methodCapacity;

    bool hasConstructor;
    bool hasDefaultValue;

    DataType *type;
    bool isStatic;
} StructNode;

typedef struct ScopedFunctionCallNode
{
    char *scopeName;       // e.g., "Math"
    char *functionName;    // e.g., "add"
    struct ASTNode **args; // Array of argument nodes
    int argCount;
    int argCapacity;
} ScopedFunctionCallNode;

typedef struct ConstructorMetaData
{
    CryoNodeType parentNodeType;
    const char *parentName;
    bool hasDefaultFlag;
} ConstructorMetaData;

typedef struct StructConstructorNode
{
    char *name;
    struct ASTNode **args;
    int argCount;
    int argCapacity;
    ConstructorMetaData *metaData;
    ASTNode *constructorBody;
} StructConstructorNode, ClassConstructorNode;

typedef struct ThisNode
{
    const char *name;
    ASTNode *object;
} ThisNode;

typedef struct PropertyAccessNode
{
    DataType *objType;
    ASTNode *object;
    const char *propertyName;
    ASTNode *property;
    int propertyIndex;
} PropertyAccessNode;

typedef struct PropertyReassignmentNode
{
    const char *name;
    ASTNode *object;
    ASTNode *value;
} PropertyReassignmentNode;

typedef struct EnumNode
{
    DataType *type;
    const char *name;
    ASTNode **properties;
    int propertyCount;
    int propertyCapacity;
} EnumNode;

typedef struct GenericDeclNode
{
    DataType *type;             // e.g., List<T>
    const char *name;           // e.g., "List"
    GenericType **properties;   // e.g., <T>
    int propertyCount;          // e.g., 1
    DataType **constraintTypes; // e.g., T extends Number
    bool hasConstraint;
} GenericDeclNode;

typedef struct GenericInstNode
{
    const char *baseName;     // e.g., "List"
    DataType **typeArguments; // e.g., <int>
    int argumentCount;
    DataType *resultType; // The concrete type after substitution
} GenericInstNode;

struct ProtectedMembers
{
    DataType *parentType;

    ASTNode **properties;
    ASTNode **methods;
    int propertyCount;
    int propertyCapacity;
    int methodCount;
    int methodCapacity;
};

struct PrivateMembers
{
    DataType *parentType;

    ASTNode **properties;
    ASTNode **methods;
    int propertyCount;
    int propertyCapacity;
    int methodCount;
    int methodCapacity;
};

struct PublicMembers
{
    DataType *parentType;

    ASTNode **properties;
    ASTNode **methods;
    int propertyCount;
    int propertyCapacity;
    int methodCount;
    int methodCapacity;
};

typedef struct ClassNode
{
    DataType *type;
    const char *name;
    ASTNode *constructor;
    int propertyCount;
    int propertyCapacity;
    int methodCount;
    int methodCapacity;
    bool hasConstructor;
    bool hasDefaultValue;
    bool isStatic;
    // For Private Members
    struct PrivateMembers *privateMembers;
    // For Public Members
    struct PublicMembers *publicMembers;
    // For Protected Members
    struct ProtectedMembers *protectedMembers;
} ClassNode;

typedef struct ObjectNode
{
    DataType *objType;
    const char *name;
    bool isNewInstance;

    ASTNode **args;
    int argCount;
    int argCapacity;

    DataType **genericTypes; // e.g., Array<Int>
    int genericCount;
    int genericCapacity;
} ObjectNode;

typedef struct NullNode
{
    DataType *type;
} NullNode;

typedef struct TypeofNode
{
    DataType *type;
    ASTNode *expression;
} TypeofNode;

typedef struct UsingNode
{
    // There will always be a constant primary module.
    const char *primaryModule;
    // There can be an array of secondary modules to go lower in the hierarchy.
    const char **secondaryModules;
    int secondaryModuleCount;
    int secondaryModuleCapacity;

    const char *filePath;
} UsingNode;

// This node will not be used in the IR generation phase.
// It is only used for the AST generation phase.
typedef struct ModuleNode
{
    const char *moduleName;
    const char *modulePath;
    const char *moduleDir;
    const char *moduleFile;
} ModuleNode;

typedef struct AnnotationNode
{
    const char *name;
    const char *value;
} AnnotationNode;

typedef struct TypeDecl
{
    const char *name;
    DataType *type;
} TypeDecl;

typedef struct TypeCast
{
    DataType *type;
    ASTNode *expression;
} TypeCast;

/// #### The ASTNode struct is the primary data structure for the Abstract Syntax Tree.
typedef struct ASTNode
{
    CryoMetaData *metaData;
    struct ASTNode *firstChild;  // First child node (for linked list structure)
    struct ASTNode *nextSibling; // Next sibling node (for linked list structure)

    union
    {
        // For Namespaces
        CryoNamespace *cryoNamespace;
        // For the main program
        CryoProgram *program;
        // For Import Statements
        CryoImportNode *import;
        // For Blocks
        CryoBlockNode *block;
        // For Scoped Function Calls
        ScopedFunctionCallNode *scopedFunctionCall;
        // For Functions
        CryoFunctionBlock *functionBlock;
        // For Externs
        ExternNode *externNode;
        // Extern Functions
        ExternFunctionNode *externFunction;
        // For Return Statements
        CryoReturnNode *returnStatement;
        // For Literals
        LiteralNode *literal;
        // For Variable Declarations
        CryoVariableNode *varDecl;
        // For Variable Names
        VariableNameNode *varName;
        // For Expressions
        CryoExpressionNode *expression;
        // For Function Declarations
        FunctionDeclNode *functionDecl;
        // For Function Calls
        FunctionCallNode *functionCall;
        // For If Statements
        IfStatementNode *ifStatement;
        // For For Statements
        ForStatementNode *forStatement;
        // For While Statements
        WhileStatementNode *whileStatement;
        // For Binary Operations
        CryoBinaryOpNode *bin_op;
        // For Unary Operations
        CryoUnaryOpNode *unary_op;
        // For Parameters
        ParamNode *paramList;
        // For Arguments
        ArgNode *argList;
        // Single Parameter Node
        CryoParameterNode *param;
        // For Arrays
        CryoArrayNode *array;
        // For Index Expressions
        IndexExprNode *indexExpr;
        // For Variable Reassignments
        VariableReassignmentNode *varReassignment;
        // For Structs
        StructNode *structNode;
        // For Properties
        PropertyNode *property;
        // For Struct Constructors
        StructConstructorNode *structConstructor;
        // For Property Access
        PropertyAccessNode *propertyAccess;
        // For This Keyword
        ThisNode *thisNode;
        // For Property Reassignment
        PropertyReassignmentNode *propertyReassignment;
        // For Methods
        MethodNode *method;
        // For Method Calls
        MethodCallNode *methodCall;
        // For Enums
        EnumNode *enumNode;
        // For Generics
        GenericDeclNode *genericDecl;
        // For Generic Instantiations
        GenericInstNode *genericInst;
        // For Classes
        ClassNode *classNode;
        // For Class Constructors
        ClassConstructorNode *classConstructor;
        // For Objects
        ObjectNode *objectNode;
        // For Null Values
        NullNode *nullNode;
        // For Typeof Expressions
        TypeofNode *typeofNode;
        // For Using Statements
        UsingNode *usingNode;
        // For Module Declarations
        ModuleNode *moduleNode;
        // For Annotations
        AnnotationNode *annotation;
        // For Type Declarations
        TypeDecl *typeDecl;
        // For Type Casts
        TypeCast *typeCast;
        void *discard;
    } data;

    _NEW_METHOD(void, print, struct ASTNode *self);
} ASTNode;

// # ============================================================ # //
// # AST Creation (./src/frontend/AST/AST.c)                      # //
// # ============================================================ # //
#ifdef __cplusplus
extern "C"
{
#endif

    void buildASTTreeLinks(ASTNode *root);
    void addChildToNode(ASTNode *parent, ASTNode *child);
    ASTNode *getFirstChild(ASTNode *node);
    ASTNode *getNextSibling(ASTNode *node);
    void traverseAST(ASTNode *node, void (*visitor)(ASTNode *));

    /**
     * AST Node Core Operations
     * Basic operations for node manipulation and management
     */
    /// @deprecated
    void printAST(
        ASTNode *node,
        int indent,
        Arena *arena);

    ASTNode *copyASTNode(
        ASTNode *node);

    // Core node management
    ASTNode *createASTNode(
        CryoNodeType type,
        Arena *arena, CompilerState *state, Lexer *lexer);

    void addChildNode(
        ASTNode *parent,
        ASTNode *child,
        Arena *arena, CompilerState *state);

    const char *getNamespaceNameFromRootNode(ASTNode *root);

    /**
     * Program Structure Nodes
     * Nodes representing high-level program structure
     */
    ASTNode *createProgramNode(
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createNamespaceNode(
        char *name,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createImportNode(
        char *module,
        char *subModule,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createExternNode(
        ASTNode *externNode,
        Arena *arena, CompilerState *state, Lexer *lexer);

    /**
     * Block Statement Nodes
     * Various types of code blocks and control structures
     */
    // Basic blocks
    ASTNode *createBlockNode(
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createFunctionBlock(
        Arena *arena, CompilerState *state, Lexer *lexer);

    void addStatementToBlock(
        ASTNode *block,
        ASTNode *statement,
        Arena *arena, CompilerState *state, Lexer *lexer);

    void addStatementToFunctionBlock(
        ASTNode *functionBlock,
        ASTNode *statement,
        Arena *arena, CompilerState *state, Lexer *lexer);

    // Control flow blocks
    ASTNode *createIfBlock(
        ASTNode *condition,
        ASTNode *then_branch,
        ASTNode *else_branch,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createForBlock(
        ASTNode *initializer,
        ASTNode *condition,
        ASTNode *increment,
        ASTNode *body,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createWhileBlock(
        ASTNode *condition,
        ASTNode *body,
        Arena *arena, CompilerState *state, Lexer *lexer);

    /**
     * Expression Nodes
     * Nodes representing various types of expressions
     */
    // Basic expressions
    ASTNode *createExpressionStatement(
        ASTNode *expression,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createBinaryExpr(
        ASTNode *left,
        ASTNode *right,
        CryoOperatorType op,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createUnaryExpr(
        CryoTokenType op,
        ASTNode *operand,
        Arena *arena, CompilerState *state, Lexer *lexer);

    // Literal expressions
    ASTNode *createLiteralIntExpr(
        int value,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createIntLiteralNode(
        int value,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createFloatLiteralNode(
        float value,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createStringLiteralNode(
        const char *value,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createBooleanLiteralNode(
        int value,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createIdentifierNode(
        char *name,
        Arena *arena, CompilerState *state, Lexer *lexer, ParsingContext *context,
        CryoGlobalSymbolTable *globalTable);

    /**
     * Function-Related Nodes
     * Nodes for function declarations, calls, and parameters
     */
    // Function definitions
    ASTNode *createFunctionNode(
        CryoVisibilityType visibility,
        char *function_name,
        ASTNode **params,
        ASTNode *function_body,
        DataType *returnType,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createExternFuncNode(
        char *function_name,
        ASTNode **params,
        DataType *returnType,
        Arena *arena, CompilerState *state, Lexer *lexer);

    void addFunctionToProgram(
        ASTNode *program,
        ASTNode *function,
        Arena *arena, CompilerState *state, Lexer *lexer);

    // Function calls and returns
    ASTNode *createFunctionCallNode(
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createScopedFunctionCall(
        Arena *arena,
        CompilerState *state,
        const char *functionName,
        Lexer *lexer);

    ASTNode *createReturnNode(
        ASTNode *returnValue,
        DataType *returnType,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createReturnExpression(
        ASTNode *returnExpression,
        DataType *returnType,
        Arena *arena, CompilerState *state, Lexer *lexer);

    // Parameters and arguments
    ASTNode *createParamListNode(
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createArgumentListNode(
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createParamNode(
        char *name,
        char *functionName,
        DataType *type,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createArgsNode(
        char *name,
        DataType *type,
        CryoNodeType nodeType,
        bool isLiteral,
        Arena *arena, CompilerState *state, Lexer *lexer);

    /**
     * Variable-Related Nodes
     * Nodes for variable declarations and operations
     */
    ASTNode *createVarDeclarationNode(
        char *var_name,
        DataType *dataType,
        ASTNode *initializer,
        bool isMutable,
        bool isGlobal,
        bool isReference,
        bool isIterator,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createVariableExpr(
        char *name,
        bool isReference,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createVarReassignment(
        char *varName,
        ASTNode *existingVarNode,
        ASTNode *newVarNode,
        Arena *arena, CompilerState *state, Lexer *lexer);

    /**
     * Struct and Object-Oriented Nodes
     * Nodes for struct definitions and object operations
     */
    // Struct definitions
    ASTNode *createStructNode(
        const char *structName,
        ASTNode **properties,
        int propertyCount,
        ASTNode *constructor,
        ASTNode **methods,
        int methodCount,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createFieldNode(
        const char *fieldName,
        DataType *type,
        const char *parentName,
        CryoNodeType parentNodeType,
        ASTNode *fieldValue,
        Arena *arena, CompilerState *state, Lexer *lexer);

    // Object operations
    ASTNode *createConstructorNode(
        char *structName,
        ASTNode *body,
        ASTNode **fields,
        int argCount,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createMethodNode(
        DataType *type,
        ASTNode *body,
        const char *methodName,
        ASTNode **args,
        int argCount,
        const char *parentName,
        bool isStatic,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createThisNode(
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createPropertyAccessNode(
        ASTNode *object,
        const char *property,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createPropertyReassignmentNode(
        ASTNode *object,
        const char *property,
        ASTNode *newValue,
        Arena *arena, CompilerState *state, Lexer *lexer);

    // Method calls and property access
    ASTNode *createStructPropertyAccessNode(
        ASTNode *object,
        ASTNode *property,
        const char *propertyName,
        DataType *type,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createMethodCallNode(
        ASTNode *accessorObj,
        DataType *returnType,
        DataType *instanceType,
        const char *methodName,
        ASTNode **args,
        int argCount,
        bool isStatic,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createTypeDeclNode(const char *typeName,
                                DataType *type, Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createTypeCastNode(DataType *type, ASTNode *expression,
                                Arena *arena, CompilerState *state, Lexer *lexer);
    /**
     * Generic Type Nodes
     * Nodes for generic type definitions and instantiations
     */
    ASTNode *createGenericDeclNode(
        DataType *type,
        const char *name,
        GenericType **properties,
        int propertyCount,
        DataType **constraintTypes,
        bool hasConstraint,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createGenericInstNode(
        const char *baseName,
        DataType **typeArguments,
        int argumentCount,
        DataType *resultType,
        Arena *arena, CompilerState *state, Lexer *lexer);

    /**
     * Array-Related Nodes
     * Nodes for array operations
     */
    ASTNode *createArrayLiteralNode(
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createIndexExprNode(
        char *arrayName,
        ASTNode *arrayRef,
        ASTNode *index,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createIfStatement(
        ASTNode *condition,
        ASTNode *then_branch,
        ASTNode *else_branch,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createForStatement(
        ASTNode *initializer,
        ASTNode *condition,
        ASTNode *increment,
        ASTNode *body,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createWhileStatement(
        ASTNode *condition,
        ASTNode *body,
        Arena *arena, CompilerState *state, Lexer *lexer);

    /**
     * Class Nodes
     * Nodes for class definitions and operations
     */

    ASTNode *createClassDeclarationNode(
        const char *className,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createClassConstructor(
        const char *className,
        ASTNode *body,
        ASTNode **fields,
        int argCount,
        Arena *arena, CompilerState *state, Lexer *lexer);

    /**
     * Object Nodes
     * Nodes for object definitions and operations
     */
    ASTNode *createObject(
        const char *objectName,
        DataType *objectType,
        bool isNew,
        ASTNode **args, int argCount,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createObjectWithGenerics(
        const char *objectName,
        DataType *objectType,
        bool isNew,
        ASTNode **args, int argCount,
        DataType **generics, int genericCount,
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createNullNode(
        Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createTypeofNode(
        ASTNode *expression,
        Arena *arena, CompilerState *state, Lexer *lexer);

    /**
     * Import / Using Nodes
     */
    ASTNode *createUsingNode(const char *primaryModule, const char *secondaryModules[],
                             int secondaryModuleCount, Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createModuleNode(const char *moduleName,
                              Arena *arena, CompilerState *state, Lexer *lexer);

    ASTNode *createAnnotationNode(const char *annotationName, const char *annotationValue,
                                  Arena *arena, CompilerState *state, Lexer *lexer);
    /**
     * String Utility Functions
     */

    char *handleStringFormatting(char *value);
    int getStringLength(char *str);

#ifdef __cplusplus
}
#endif

// # ============================================================ #
// # Node Containers (./src/frontend/AST/ASTContainers.c)         #
// # ============================================================ #

CryoNamespace *createCryoNamespaceNodeContainer(Arena *arena, CompilerState *state);
CryoProgram *createCryoProgramContainer(Arena *arena, CompilerState *state);
CryoBlockNode *createCryoBlockNodeContainer(Arena *arena, CompilerState *state);
CryoFunctionBlock *createCryoFunctionBlockContainer(Arena *arena, CompilerState *state);
CryoModule *createCryoModuleContainer(Arena *arena, CompilerState *state);
CryoMetaData *createMetaDataContainer(Arena *arena, CompilerState *state);
CryoImportNode *createCryoImportNodeContainer(Arena *arena, CompilerState *state);
CryoScope *createCryoScopeContainer(Arena *arena, CompilerState *state);
ExternNode *createExternNodeContainer(CryoNodeType type, Arena *arena, CompilerState *state);
FunctionDeclNode *createFunctionNodeContainer(Arena *arena, CompilerState *state);
FunctionCallNode *createFunctionCallNodeContainer(Arena *arena, CompilerState *state);
LiteralNode *createLiteralNodeContainer(Arena *arena, CompilerState *state);
IfStatementNode *createIfStatementContainer(Arena *arena, CompilerState *state);
ForStatementNode *createForStatementNodeContainer(Arena *arena, CompilerState *state);
WhileStatementNode *createWhileStatementNodeContainer(Arena *arena, CompilerState *state);
CryoExpressionNode *createExpressionNodeContainer(Arena *arena, CompilerState *state);
CryoVariableNode *createVariableNodeContainer(Arena *arena, CompilerState *state);
VariableNameNode *createVariableNameNodeContainer(char *varName, Arena *arena, CompilerState *state);
CryoParameterNode *createParameterNodeContainer(Arena *arena, CompilerState *state);
ParamNode *createParamNodeContainer(Arena *arena, CompilerState *state);
ArgNode *createArgNodeContainer(Arena *arena, CompilerState *state);
CryoReturnNode *createReturnNodeContainer(Arena *arena, CompilerState *state);
CryoBinaryOpNode *createBinaryOpNodeContainer(Arena *arena, CompilerState *state);
CryoUnaryOpNode *createUnaryOpNodeContainer(Arena *arena, CompilerState *state);
CryoArrayNode *createArrayNodeContainer(Arena *arena, CompilerState *state);
IndexExprNode *createIndexExprNodeContainer(Arena *arena, CompilerState *state);
VariableReassignmentNode *createVariableReassignmentNodeContainer(Arena *arena, CompilerState *state);
StructNode *createStructNodeContainer(Arena *arena, CompilerState *state);
PropertyNode *createPropertyNodeContainer(Arena *arena, CompilerState *state);
ScopedFunctionCallNode *createScopedFunctionCallNode(Arena *arena, CompilerState *state);
StructConstructorNode *createStructConstructorNodeContainer(Arena *arena, CompilerState *state);
ConstructorMetaData *createConstructorMetaDataContainer(Arena *arena, CompilerState *state);
PropertyAccessNode *createPropertyAccessNodeContainer(Arena *arena, CompilerState *state);
ThisNode *createThisNodeContainer(Arena *arena, CompilerState *state);
PropertyReassignmentNode *createPropertyReassignmentNodeContainer(Arena *arena, CompilerState *state);
MethodNode *createMethodNodeContainer(Arena *arena, CompilerState *state);
MethodCallNode *createMethodCallNodeContainer(Arena *arena, CompilerState *state);
GenericDeclNode *createGenericDeclNodeContainer(Arena *arena, CompilerState *state);
GenericInstNode *createGenericInstNodeContainer(Arena *arena, CompilerState *state);
void *createMembersContainer(Arena *arena, CompilerState *state);
ClassNode *createClassNodeContainer(Arena *arena, CompilerState *state);
ClassConstructorNode *createClassConstructorNodeContainer(Arena *arena, CompilerState *state);
ObjectNode *createObjectNodeContainer(Arena *arena, CompilerState *state);
NullNode *createNullNodeContainer(Arena *arena, CompilerState *state);
TypeofNode *createTypeofNodeContainer(Arena *arena, CompilerState *state);
UsingNode *createUsingNodeContainer(Arena *arena, CompilerState *state);
ModuleNode *createModuleNodeContainer(Arena *arena, CompilerState *state);
AnnotationNode *createAnnotationNodeContainer(Arena *arena, CompilerState *state);
TypeDecl *createTypeDeclContainer(Arena *arena, CompilerState *state);
TypeCast *createTypeCastContainer(Arena *arena, CompilerState *state);

// # ============================================================ #
// # AST Debug Output (./src/frontend/AST/debugOutputAST.c)       #
// # ============================================================ #

typedef struct ASTDebugNode
{
    const char *nodeType;
    const char *nodeName;
    DataType *dataType;
    int line;
    int column;
    struct ASTDebugNode *children;
    int childCount;
    int indent;
    const char *namespaceName;
    ASTNode *sourceNode;
    ASTNode *value;
    ASTNode **args;
    int argCount;
} ASTDebugNode;

typedef struct DebugASTOutput
{
    const char *fileName;
    const char *short_fileName;
    const char *filePath;
    const char *fileExt;
    const char *cwd;
    ASTDebugNode *nodes;
    int nodeCount;
} DebugASTOutput;

int initASTDebugOutput(ASTNode *root, CompilerSettings *settings);
void initASTConsoleOutput(ASTNode *root, const char *filePath);
void logASTNodeDebugView(ASTNode *node);

DebugASTOutput *createDebugASTOutput(const char *fileName, const char *filePath, const char *fileExt, const char *cwd);
ASTDebugNode *createASTDebugNode(const char *nodeType, const char *nodeName, DataType *dataType, int line, int column, int indent, ASTNode *sourceNode);
void createASTDebugView(ASTNode *node, DebugASTOutput *output, int indentLevel);
DebugASTOutput *addDebugNodesToOutput(ASTDebugNode *node, DebugASTOutput *output);
void createASTDebugOutputFile(DebugASTOutput *output);
void removePrevASTOutput(const char *filePath);
char *seekNamespaceName(ASTNode *node);
bool propHasDefault(PropertyNode *prop);

// Output File Buffer
char *getASTBuffer(DebugASTOutput *output, bool console);
char *logASTBuffer(DebugASTOutput *output, bool console);

char *ASTNodeValueBuffer(ASTNode *node);
void logASTNode(ASTNode *node);
char *formattedNewKeyword(void);
char *printFormattedDataTypeArray(DataType **typeArray, int typeCount);

// Formatting Functions
char *formatASTNode(ASTDebugNode *node, DebugASTOutput *output, int indentLevel, bool console);
char *formatProgramNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatFunctionDeclNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatParamListNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatBlockNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatVarDeclNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatExpressionNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatLiteralExprNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatReturnStatementNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatFunctionCallNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatParamNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatPropertyNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatVarNameNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatStructNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatExternFunctionNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatFunctionBlock(ASTDebugNode *node, DebugASTOutput *output);
char *formatStructConstructor(ASTDebugNode *node, DebugASTOutput *output);
char *formatThisNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatThisAssignmentNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatPropertyAssignmentNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatMethodNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatIntLiteralNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatStringLiteralNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatBooleanLiteralNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatMethodCallNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatBinOpNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatGenericDeclNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatGenericInstNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatClassNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatAccessControlNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatArgListNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatObjectInstNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatUnaryOpNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatTypeNode(ASTDebugNode *node, DebugASTOutput *output);

char *CONSOLE_formatASTNode(ASTDebugNode *node, DebugASTOutput *output, int indentLevel);
char *CONSOLE_formatProgramNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatFunctionDeclNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatParamListNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatBlockNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatVarDeclNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatExpressionNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatLiteralExprNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatReturnStatementNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatFunctionCallNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatParamNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatPropertyNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatVarNameNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatStructNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatExternFunctionNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatFunctionBlock(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatStructConstructor(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatThisNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatThisAssignmentNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatPropertyAssignmentNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatMethodNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatIntLiteralNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatStringLiteralNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatBooleanLiteralNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatMethodCallNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatBinOpNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatGenericDeclNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatGenericInstNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatClassNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatAccessControlNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatArgListNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatObjectInstNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatUnaryOpNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatTypeNode(ASTDebugNode *node, DebugASTOutput *output);

#endif // AST_H
