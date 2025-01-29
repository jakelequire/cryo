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
#include "diagnostics/diagnostics.h"
#include "tools/logger/logger_config.h"

GlobalDiagnosticsManager *g_diagnosticsManager = NULL;

void initGlobalDiagnosticsManager(void)
{
    if (g_diagnosticsManager == NULL)
    {
        g_diagnosticsManager = (GlobalDiagnosticsManager *)malloc(sizeof(GlobalDiagnosticsManager));
        GDM->errorCount = 0;
        GDM->errors = NULL;
        GDM->stackTrace = newStackTrace();
        GDM->frontendState = newFrontendState();

        // Methods
        GDM->addLexer = addLexer;
        GDM->printStackTrace = print_stack_trace;
        GDM->createStackFrame = create_stack_frame;
    }
}
