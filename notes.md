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
