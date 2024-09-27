

#include "cpp/codegen.h"

namespace Cryo
{
    void Structs::handleStructDeclaration(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Structs", "Handling Struct Declaration");

        DEBUG_BREAKPOINT;
    }

}
