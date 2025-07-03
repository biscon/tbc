//
// Created by bison on 01-07-25.
//

#ifndef SANDBOX_PORTRAITS_H
#define SANDBOX_PORTRAITS_H

#include "data/GameData.h"

#define PORTRAIT_DEFAULT        0


inline void DrawPortrait(GameData& data, int x, int y, Color tint, int portrait) {
    int sheetId = data.ui.portraitSpriteSheet;
    Texture2D tex = data.spriteData.sheet.texture[sheetId];
    Rectangle srcRect = data.spriteData.sheet.frameRects[sheetId][portrait];
    DrawTextureRec(tex, srcRect, { (float) x, (float) y}, tint);
}


#endif
