
## Phases of Compilation
The compilation process for the Cryo programming language is divided into two main phases:
1. **Phase 1: C/C++ Implementation**
   - This phase involves implementing the core runtime and basic standard library in C/C++. The goal is to create a solid foundation for the Cryo language.
2. **Phase 2: Cryo Implementation** 
   - In this phase, the core runtime and basic standard library will be re-implemented in Cryo itself. This will allow for a more idiomatic use of the language and take advantage of its features.

```graph TD
    A[Phase 1: C/C++ Implementation] --> B[Core Runtime]
    A --> C[Basic Standard Library]
    B --> D[Phase 2: Cryo Implementation]
    C --> D
    D --> E[Full Standard Library in Cryo]
```

Pseudo code for the compiler:
```
class CryoCompiler {
    void initialize() {
        // 1. Compile runtime
        auto runtimeModule = compileRuntime();
        
        // 2. Compile standard library
        auto stdlibModule = compileStdLib();
        
        // 3. Link them together
        linkModules(runtimeModule, stdlibModule);
        
        // Store for use when compiling user code
        this->baseModule = runtimeModule;
    }
    
    void compileUserCode(const std::string& source) {
        // Create new module for user code
        auto userModule = createModule("user");
        
        // Link with base module (runtime + stdlib)
        linkModules(userModule, this->baseModule);
        
        // Compile user code
        compile(source, userModule);
    }
};
```
This core library provides:

Runtime Linkage

External function declarations that link to C++ runtime
Basic memory management wrappers
Fundamental Types

Safe wrappers around allocation
String memory management
Key points about this file:

It should be the first standard library file compiled
Contains only the most fundamental types and operations
Everything else in the standard library will build on this
Keeps runtime dependencies minimal and explicit
You would then build other standard library modules on top of this, like:

std/collections/ - For data structures
std/io/ - For input/output operations
std/sys/ - For system operations
Each of these would import core.cryo as their foundation.
