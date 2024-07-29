
// --------------------------------------------------------------------
// Notes on LLVM Type creation and using the `Type` interface 
LLVMContext &Context = TheModule->getContext(); 

// Integer types
llvm::Type *Int1Ty = Type::getInt1Ty(Context);    // i1
llvm::Type *Int8Ty = Type::getInt8Ty(Context);    // i8
llvm::Type *Int16Ty = Type::getInt16Ty(Context);  // i16
llvm::Type *Int32Ty = Type::getInt32Ty(Context);  // i32
llvm::Type *Int64Ty = Type::getInt64Ty(Context);  // i64
llvm::Type *Int128Ty = Type::getInt128Ty(Context);// i128

// Floating-point types
llvm::Type *HalfTy = Type::getHalfTy(Context);    // half
llvm::Type *FloatTy = Type::getFloatTy(Context);  // float
llvm::Type *DoubleTy = Type::getDoubleTy(Context);// double
llvm::Type *FP128Ty = Type::getFP128Ty(Context);  // fp128

// Void type
llvm::Type *VoidTy = Type::getVoidTy(Context);

// Label type (for basic blocks)
llvm::Type *LabelTy = Type::getLabelTy(Context);

// Pointer types
llvm::Type *Int8PtrTy = Type::getInt8PtrTy(Context);
llvm::Type *Int32PtrTy = Type::getInt32PtrTy(Context);
llvm::Type *VoidPtrTy = Type::getInt8PtrTy(Context);  // void* is usually represented as i8*

// Array types
llvm::Type *ArrayTy = ArrayType::get(Int32Ty, 10);  // [10 x i32]

// Vector types
llvm::Type *VectorTy = VectorType::get(FloatTy, 4);  // <4 x float>

// Struct types
std::vector<Type*> StructElements = {Int32Ty, FloatTy, Int8PtrTy};
llvm::StructType *StructTy = StructType::get(Context, StructElements);

// Function types
std::vector<Type*> ParamTypes = {Int32Ty, FloatTy};
llvm::FunctionType *FuncTy = FunctionType::get(Int32Ty, ParamTypes, false);

// Opaque struct type
llvm::StructType *OpaqueStructTy = StructType::create(Context, "MyStruct");

// Named struct type
llvm::StructType *NamedStructTy = StructType::create(Context, StructElements, "MyNamedStruct");

// Metadata type
llvm::Type *MetadataTy = Type::getMetadataTy(Context);

// Token type (for certain intrinsics)
llvm::Type *TokenTy = Type::getTokenTy(Context); 
// --------------------------------------------------------------------


/**
 * Helper Function to view LLVM types more clearly for debugging.
 */
std::string CodeGen::LLVMTypeToString(llvm::Type* type) {
    std::string typeStr;
    llvm::raw_string_ostream rso(typeStr);
    type->print(rso);
    return rso.str();
}


llvm::Type* CodeGen::createLLVMConstantType(CryoDataType type) {
    if (type == DATA_TYPE_UNKNOWN) {
        std::cerr << "[Types] Error: Unknown type\n";
        return nullptr;
    }

    switch(type) {
        case DATA_TYPE_INT:
            return llvm::Type::getInt32Ty(context);
        case DATA_TYPE_FLOAT:
            return llvm::Type::getFloatTy(context);
        case DATA_TYPE_STRING:
            return llvm::Type::getInt8Ty(context); // String is represented as i8
        case DATA_TYPE_BOOLEAN:
            return llvm::Type::getInt1Ty(context);
        case DATA_TYPE_VOID:
            return llvm::Type::getVoidTy(context);
        default:
            std::cerr << "[Types] Error: Unsupported type\n";
            return nullptr;
    }
}

llvm::PointerType* CodeGen::createLLVMPointerType(CryoDataType type) {
    llvm::Type* baseType = createLLVMConstantType(type);
    if (!baseType) {
        return nullptr;
    }
    return llvm::PointerType::getUnqual(baseType);
}

llvm::ArrayType* CodeGen::createLLVMArrayType(CryoDataType elementType, unsigned int size) {
    llvm::Type* baseType = createLLVMConstantType(elementType);
    if (!baseType) {
        return nullptr;
    }
    return llvm::ArrayType::get(baseType, size);
}

llvm::StructType* CodeGen::createLLVMStructType(const std::vector<CryoDataType>& memberTypes, const std::string& name = "") {
    std::vector<llvm::Type*> llvmTypes;
    for (const auto& type : memberTypes) {
        llvm::Type* memberType = createLLVMConstantType(type);
        if (!memberType) {
            return nullptr;
        }
        llvmTypes.push_back(memberType);
    }
    
    if (name.empty()) {
        return llvm::StructType::get(context, llvmTypes);
    } else {
        return llvm::StructType::create(context, llvmTypes, name);
    }
}


llvm::FunctionType* CodeGen::createLLVMFunctionType(CryoDataType returnType, const std::vector<CryoDataType>& paramTypes, bool isVarArg = false) {
    llvm::Type* llvmReturnType = createLLVMConstantType(returnType);
    if (!llvmReturnType) {
        return nullptr;
    }

    std::vector<llvm::Type*> llvmParamTypes;
    for (const auto& type : paramTypes) {
        llvm::Type* paramType = createLLVMConstantType(type);
        if (!paramType) {
            return nullptr;
        }
        llvmParamTypes.push_back(paramType);
    }

    return llvm::FunctionType::get(llvmReturnType, llvmParamTypes, isVarArg);
}



