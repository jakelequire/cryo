// Type.h
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"

class Type;
using TypePtr = std::shared_ptr<Type>;

class Type : public std::enable_shared_from_this<Type>
{
public:
    enum class Kind
    {
        Primitive,
        Array,
        Struct,
        Union,
        Function,
        Generic,
        Any,
        Nullable,
        Reference,
        Pointer
    };

    Type(Kind kind, const std::string &name) : kind(kind), name(name) {}
    virtual ~Type() = default;

    Kind getKind() const { return kind; }
    const std::string &getName() const { return name; }

    virtual llvm::Type *toLLVMType(llvm::LLVMContext &context) const = 0;
    virtual std::string toString() const = 0;
    virtual bool isEquivalentTo(const TypePtr &other) const = 0;
    virtual size_t getSize() const = 0;
    virtual TypePtr clone() const = 0;

protected:
    Kind kind;
    std::string name;
};

class PrimitiveType : public Type
{
public:
    PrimitiveType(const std::string &name, size_t size) : Type(Kind::Primitive, name), size(size) {}

    llvm::Type *toLLVMType(llvm::LLVMContext &context) const override
    {
        if (name == "int")
            return llvm::Type::getInt32Ty(context);
        if (name == "long")
            return llvm::Type::getInt64Ty(context);
        if (name == "float")
            return llvm::Type::getFloatTy(context);
        if (name == "double")
            return llvm::Type::getDoubleTy(context);
        if (name == "bool")
            return llvm::Type::getInt1Ty(context);
        if (name == "char")
            return llvm::Type::getInt8Ty(context);
        if (name == "string")
            return llvm::Type::getInt8Ty(context);
        return nullptr;
    }

    std::string toString() const override { return name; }
    bool isEquivalentTo(const TypePtr &other) const override
    {
        if (auto primOther = std::dynamic_pointer_cast<PrimitiveType>(other))
        {
            return name == primOther->name && size == primOther->size;
        }
        return false;
    }
    size_t getSize() const override { return size; }
    TypePtr clone() const override { return std::make_shared<PrimitiveType>(*this); }

private:
    // The private members are implementation details

    // These are the sizes in bytes
    size_t size;
};

class ArrayType : public Type
{
public:
    ArrayType(TypePtr elementType, std::optional<size_t> size = std::nullopt)
        : Type(Kind::Array, "array"), elementType(elementType), size(size) {}

    llvm::Type *toLLVMType(llvm::LLVMContext &context) const override
    {
        llvm::Type *llvmElementType = elementType->toLLVMType(context);
        return size
                   ? llvm::ArrayType::get(llvmElementType, *size)
                   : llvm::dyn_cast<llvm::Type>(llvm::PointerType::get(llvmElementType, 0));
    }

    std::string toString() const override
    {
        return elementType->toString() + "[" + (size ? std::to_string(*size) : "") + "]";
    }

    bool isEquivalentTo(const TypePtr &other) const override
    {
        if (auto arrOther = std::dynamic_pointer_cast<ArrayType>(other))
        {
            return elementType->isEquivalentTo(arrOther->elementType) && size == arrOther->size;
        }
        return false;
    }

    size_t getSize() const override { return size ? *size * elementType->getSize() : sizeof(void *); }
    TypePtr clone() const override { return std::make_shared<ArrayType>(*this); }

    TypePtr getElementType() const { return elementType; }
    std::optional<size_t> getArraySize() const { return size; }

private:
    TypePtr elementType;
    std::optional<size_t> size;
};

class StructType : public Type
{
public:
    StructType(const std::string &name, const std::vector<std::pair<std::string, TypePtr>> &members)
        : Type(Kind::Struct, name), members(members) {}

    llvm::Type *toLLVMType(llvm::LLVMContext &context) const override
    {
        std::vector<llvm::Type *> llvmMembers;
        for (const auto &[_, memberType] : members)
        {
            llvmMembers.push_back(memberType->toLLVMType(context));
        }
        return llvm::StructType::create(context, llvmMembers, name);
    }

    std::string toString() const override
    {
        std::string result = "struct " + name + " { ";
        for (size_t i = 0; i < members.size(); ++i)
        {
            if (i > 0)
                result += ", ";
            result += members[i].second->toString() + " " + members[i].first;
        }
        result += " }";
        return result;
    }

    bool isEquivalentTo(const TypePtr &other) const override
    {
        if (auto structOther = std::dynamic_pointer_cast<StructType>(other))
        {
            if (members.size() != structOther->members.size())
                return false;
            for (size_t i = 0; i < members.size(); ++i)
            {
                if (members[i].first != structOther->members[i].first ||
                    !members[i].second->isEquivalentTo(structOther->members[i].second))
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    size_t getSize() const override
    {
        size_t totalSize = 0;
        for (const auto &[_, memberType] : members)
        {
            totalSize += memberType->getSize();
        }
        return totalSize;
    }

    TypePtr clone() const override { return std::make_shared<StructType>(*this); }

    const std::vector<std::pair<std::string, TypePtr>> &getMembers() const { return members; }

private:
    std::vector<std::pair<std::string, TypePtr>> members;
};

class UnionType : public Type
{
public:
    UnionType(const std::string &name, const std::vector<std::pair<std::string, TypePtr>> &members)
        : Type(Kind::Union, name), members(members) {}

    llvm::Type *toLLVMType(llvm::LLVMContext &context) const override
    {
        std::vector<llvm::Type *> llvmMembers;
        for (const auto &[_, memberType] : members)
        {
            llvmMembers.push_back(memberType->toLLVMType(context));
        }
        return llvm::StructType::create(context, llvmMembers, name);
    }

    std::string toString() const override
    {
        std::string result = "union " + name + " { ";
        for (size_t i = 0; i < members.size(); ++i)
        {
            if (i > 0)
                result += " | ";
            result += members[i].second->toString() + " " + members[i].first;
        }
        result += " }";
        return result;
    }

    bool isEquivalentTo(const TypePtr &other) const override
    {
        if (auto unionOther = std::dynamic_pointer_cast<UnionType>(other))
        {
            if (members.size() != unionOther->members.size())
                return false;
            for (size_t i = 0; i < members.size(); ++i)
            {
                if (members[i].first != unionOther->members[i].first ||
                    !members[i].second->isEquivalentTo(unionOther->members[i].second))
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    size_t getSize() const override
    {
        size_t maxSize = 0;
        for (const auto &[_, memberType] : members)
        {
            maxSize = std::max(maxSize, memberType->getSize());
        }
        return maxSize;
    }

    TypePtr clone() const override { return std::make_shared<UnionType>(*this); }

    const std::vector<std::pair<std::string, TypePtr>> &getMembers() const { return members; }

private:
    std::vector<std::pair<std::string, TypePtr>> members;
};

class FunctionType : public Type
{
public:
    FunctionType(TypePtr returnType, const std::vector<TypePtr> &paramTypes)
        : Type(Kind::Function, "function"), returnType(returnType), paramTypes(paramTypes) {}

    llvm::Type *toLLVMType(llvm::LLVMContext &context) const override
    {
        std::vector<llvm::Type *> llvmParamTypes;
        for (const auto &paramType : paramTypes)
        {
            llvmParamTypes.push_back(paramType->toLLVMType(context));
        }
        return llvm::FunctionType::get(returnType->toLLVMType(context), llvmParamTypes, false);
    }

    std::string toString() const override
    {
        std::string result = "fn(";
        for (size_t i = 0; i < paramTypes.size(); ++i)
        {
            if (i > 0)
                result += ", ";
            result += paramTypes[i]->toString();
        }
        result += ") -> " + returnType->toString();
        return result;
    }

    bool isEquivalentTo(const TypePtr &other) const override
    {
        if (auto funcOther = std::dynamic_pointer_cast<FunctionType>(other))
        {
            if (!returnType->isEquivalentTo(funcOther->returnType))
                return false;
            if (paramTypes.size() != funcOther->paramTypes.size())
                return false;
            for (size_t i = 0; i < paramTypes.size(); ++i)
            {
                if (!paramTypes[i]->isEquivalentTo(funcOther->paramTypes[i]))
                    return false;
            }
            return true;
        }
        return false;
    }

    size_t getSize() const override { return sizeof(void *); } // Function pointers are typically pointer-sized
    TypePtr clone() const override { return std::make_shared<FunctionType>(*this); }

    TypePtr getReturnType() const { return returnType; }
    const std::vector<TypePtr> &getParamTypes() const { return paramTypes; }

private:
    TypePtr returnType;
    std::vector<TypePtr> paramTypes;
};

class GenericType : public Type
{
public:
    GenericType(const std::string &name, const std::vector<std::string> &typeParams)
        : Type(Kind::Generic, name), typeParams(typeParams) {}

    llvm::Type *toLLVMType(llvm::LLVMContext &context) const override
    {
        // Generic types are resolved at compile-time, so this should not be called directly
        return nullptr;
    }

    std::string toString() const override
    {
        std::string result = name + "<";
        for (size_t i = 0; i < typeParams.size(); ++i)
        {
            if (i > 0)
                result += ", ";
            result += typeParams[i];
        }
        result += ">";
        return result;
    }

    bool isEquivalentTo(const TypePtr &other) const override
    {
        if (auto genOther = std::dynamic_pointer_cast<GenericType>(other))
        {
            return name == genOther->name && typeParams == genOther->typeParams;
        }
        return false;
    }

    size_t getSize() const override { return 0; } // Size is unknown until instantiated
    TypePtr clone() const override { return std::make_shared<GenericType>(*this); }

    const std::vector<std::string> &getTypeParams() const { return typeParams; }

private:
    std::vector<std::string> typeParams;
};

class AnyType : public Type
{
public:
    AnyType() : Type(Kind::Any, "any") {}

    llvm::Type *toLLVMType(llvm::LLVMContext &context) const override
    {
        std::vector<llvm::Type *> members = {
            llvm::Type::getInt32Ty(context), // Type tag
            llvm::Type::getInt8Ty(context)   // Value pointer
        };
        return llvm::StructType::get(context, members);
    }

    std::string toString() const override { return "any"; }
    bool isEquivalentTo(const TypePtr &other) const override
    {
        return std::dynamic_pointer_cast<AnyType>(other) != nullptr;
    }
    size_t getSize() const override { return sizeof(int32_t) + sizeof(void *); }
    TypePtr clone() const override { return std::make_shared<AnyType>(*this); }
};

class NullableType : public Type
{
public:
    NullableType(TypePtr baseType) : Type(Kind::Nullable, "nullable"), baseType(baseType) {}

    llvm::Type *toLLVMType(llvm::LLVMContext &context) const override
    {
        return llvm::PointerType::get(baseType->toLLVMType(context), 0);
    }

    std::string toString() const override { return baseType->toString() + "?"; }
    bool isEquivalentTo(const TypePtr &other) const override
    {
        if (auto nullOther = std::dynamic_pointer_cast<NullableType>(other))
        {
            return baseType->isEquivalentTo(nullOther->baseType);
        }
        return false;
    }
    size_t getSize() const override { return sizeof(void *); }
    TypePtr clone() const override { return std::make_shared<NullableType>(*this); }

    TypePtr getBaseType() const { return baseType; }

private:
    TypePtr baseType;
};

class ReferenceType : public Type
{
public:
    ReferenceType(TypePtr referencedType) : Type(Kind::Reference, "reference"), referencedType(referencedType) {}

    llvm::Type *toLLVMType(llvm::LLVMContext &context) const override
    {
        return llvm::PointerType::get(referencedType->toLLVMType(context), 0);
    }

    std::string toString() const override { return referencedType->toString() + "&"; }
    bool isEquivalentTo(const TypePtr &other) const override
    {
        if (auto refOther = std::dynamic_pointer_cast<ReferenceType>(other))
        {
            return referencedType->isEquivalentTo(refOther->referencedType);
        }
        return false;
    }
    size_t getSize() const override { return sizeof(void *); }
    TypePtr clone() const override { return std::make_shared<ReferenceType>(*this); }

    TypePtr getReferencedType() const { return referencedType; }

private:
    TypePtr referencedType;
};

class PointerType : public Type
{
public:
    PointerType(TypePtr pointeeType) : Type(Kind::Pointer, "pointer"), pointeeType(pointeeType) {}

    llvm::Type *toLLVMType(llvm::LLVMContext &context) const override
    {
        return llvm::PointerType::get(pointeeType->toLLVMType(context), 0);
    }

    std::string toString() const override { return pointeeType->toString() + "*"; }
    bool isEquivalentTo(const TypePtr &other) const override
    {
        if (auto ptrOther = std::dynamic_pointer_cast<PointerType>(other))
        {
            return pointeeType->isEquivalentTo(ptrOther->pointeeType);
        }
        return false;
    }
    size_t getSize() const override { return sizeof(void *); }
    TypePtr clone() const override { return std::make_shared<PointerType>(*this); }

    TypePtr getPointeeType() const { return pointeeType; }

private:
    TypePtr pointeeType;
};

class TypeSystem
{
public:
    TypeSystem()
    {
        // Register built-in types
        registerType(std::make_shared<PrimitiveType>("int", 4));
        registerType(std::make_shared<PrimitiveType>("long", 8));
        registerType(std::make_shared<PrimitiveType>("float", 4));
        registerType(std::make_shared<PrimitiveType>("double", 8));
        registerType(std::make_shared<PrimitiveType>("bool", 1));
        registerType(std::make_shared<PrimitiveType>("char", 1));
        registerType(std::make_shared<PrimitiveType>("string", sizeof(char *)));
        registerType(std::make_shared<AnyType>());
    }

    void registerType(TypePtr type)
    {
        types[type->getName()] = type;
    }

    TypePtr getType(const std::string &name) const
    {
        auto it = types.find(name);
        return it != types.end() ? it->second : nullptr;
    }

    TypePtr createArrayType(TypePtr elementType, std::optional<size_t> size = std::nullopt)
    {
        return std::make_shared<ArrayType>(elementType, size);
    }

    TypePtr createStructType(const std::string &name, const std::vector<std::pair<std::string, TypePtr>> &members)
    {
        auto structType = std::make_shared<StructType>(name, members);
        registerType(structType);
        return structType;
    }

    TypePtr createUnionType(const std::string &name, const std::vector<std::pair<std::string, TypePtr>> &members)
    {
        auto unionType = std::make_shared<UnionType>(name, members);
        registerType(unionType);
        return unionType;
    }

    TypePtr createFunctionType(TypePtr returnType, const std::vector<TypePtr> &paramTypes)
    {
        return std::make_shared<FunctionType>(returnType, paramTypes);
    }

    TypePtr createGenericType(const std::string &name, const std::vector<std::string> &typeParams)
    {
        auto genericType = std::make_shared<GenericType>(name, typeParams);
        registerType(genericType);
        return genericType;
    }

    TypePtr createNullableType(TypePtr baseType)
    {
        return std::make_shared<NullableType>(baseType);
    }

    TypePtr createReferenceType(TypePtr referencedType)
    {
        return std::make_shared<ReferenceType>(referencedType);
    }

    TypePtr createPointerType(TypePtr pointeeType)
    {
        return std::make_shared<PointerType>(pointeeType);
    }

    bool areTypesCompatible(TypePtr type1, TypePtr type2) const
    {
        if (type1->isEquivalentTo(type2))
            return true;
        if (std::dynamic_pointer_cast<AnyType>(type1) || std::dynamic_pointer_cast<AnyType>(type2))
            return true;
        // Add more complex compatibility rules here
        return false;
    }

    TypePtr getCommonType(TypePtr type1, TypePtr type2) const
    {
        if (type1->isEquivalentTo(type2))
            return type1;
        if (std::dynamic_pointer_cast<AnyType>(type1))
            return type2;
        if (std::dynamic_pointer_cast<AnyType>(type2))
            return type1;
        // Add more complex type promotion rules here
        return nullptr;
    }

private:
    std::unordered_map<std::string, TypePtr> types;
};

// Usage example
class TypeChecker
{
public:
    TypeChecker(TypeSystem &typeSystem) : typeSystem(typeSystem) {}

    TypePtr checkBinaryOperation(TypePtr leftType, TypePtr rightType, std::string op)
    {
        if (op == "+" || op == "-" || op == "*" || op == "/")
        {
            if (typeSystem.areTypesCompatible(leftType, rightType))
            {
                return typeSystem.getCommonType(leftType, rightType);
            }
        }
        // Add more operation checks
        return nullptr; // Incompatible types
    }

    TypePtr checkFunctionCall(TypePtr functionType, const std::vector<TypePtr> &argTypes)
    {
        if (auto funcType = std::dynamic_pointer_cast<FunctionType>(functionType))
        {
            const auto &paramTypes = funcType->getParamTypes();
            if (paramTypes.size() != argTypes.size())
                return nullptr;
            for (size_t i = 0; i < paramTypes.size(); ++i)
            {
                if (!typeSystem.areTypesCompatible(paramTypes[i], argTypes[i]))
                    return nullptr;
            }
            return funcType->getReturnType();
        }
        return nullptr;
    }

    // Add more type checking methods as needed

private:
    TypeSystem &typeSystem;
};
