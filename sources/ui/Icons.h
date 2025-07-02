//
// Created by bison on 01-07-25.
//

#ifndef SANDBOX_ICONS_H
#define SANDBOX_ICONS_H

#include "data/GameData.h"

#define ICON_WEAPON_SWAP        0
#define ICON_END_TURN           1
#define ICON_MOVE               2
#define ICON_RELOAD             3
#define ICON_ATTACK             4

inline void DrawIcon(GameData& data, int x, int y, Color tint, int icon) {
    int sheetId = data.ui.iconSpriteSheet;
    Texture2D tex = data.spriteData.sheet.texture[sheetId];
    Rectangle srcRect = data.spriteData.sheet.frameRects[sheetId][icon];
    DrawTextureRec(tex, srcRect, { (float) x, (float) y}, tint);
}


#endif //SANDBOX_ICONS_H
