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
#ifndef VISUAL_DEBUG_H
#define VISUAL_DEBUG_H
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <cmath>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>

#include <SFML/Graphics.hpp>

#include "compiler/ast.h"
#include "compiler/token.h"
#include "common/common.h"
#include "utils/fs.h"

typedef struct ASTNode ASTNode;

struct VisualNode
{
    sf::RectangleShape shape;
    sf::Text text;
    float x;
    float y;
    std::vector<VisualNode *> children;
};

typedef struct iNode
{
    CryoNodeType nodeType;
    CryoDataType dataType;
    std::string name;
    std::string value;
} iNode;

VisualNode* createVisualTree(ASTNode* astNode, sf::Font& font, int depth);
void layoutTree(VisualNode *root, float x, float y, float availableWidth);
void drawTree(sf::RenderWindow &window, VisualNode *root);
void renderWindow(ASTNode *programNode);

#endif // VISUAL_DEBUG_H