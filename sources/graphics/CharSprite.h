//
// Created by bison on 26-12-25.
//

#ifndef SANDBOX_CHARSPRITE_H
#define SANDBOX_CHARSPRITE_H

#include <string>
#include "data/SpriteData.h"

void InitCharSprite(SpriteData& spriteData, CharSprite &sprite, const std::string& spriteTemplate);
void UpdateCharSprite(SpriteData& spriteData, CharSprite& sprite, float deltaTime);

#endif //SANDBOX_CHARSPRITE_H
