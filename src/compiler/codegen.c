#include "codegen.h"
#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>



// Function to initialize LLVM and create a simple function
void generate_simple_function() {
    LLVMModuleRef mod = LLVMModuleCreateWithName("simple_module");
    LLVMBuilderRef builder = LLVMCreateBuilder();
    LLVMContextRef context = LLVMContextCreate();
    
    // Define the return type and function signature
    LLVMTypeRef ret_type = LLVMInt32TypeInContext(context);
    LLVMTypeRef func_type = LLVMFunctionType(ret_type, NULL, 0, 0);
    LLVMValueRef func = LLVMAddFunction(mod, "simple_func", func_type);
    
    // Create a basic block and position the builder
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(context, func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
    
    // Return the integer 1
    LLVMValueRef ret_val = LLVMConstInt(LLVMInt32Type(), 1, 0);
    LLVMBuildRet(builder, ret_val);
    
    // Save the IR to a file
    char *error = NULL;
    if (LLVMPrintModuleToFile(mod, "simple_module.ll", &error) != 0) {
        fprintf(stderr, "Error writing to file: %s\n", error);
        LLVMDisposeMessage(error);
    }
    
    // Clean up
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(mod);
    LLVMContextDispose(context);
}

int generate_code() {
    // Initialize LLVM
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();
    
    // Generate the simple function
    generate_simple_function();
    
    return 0;
}
