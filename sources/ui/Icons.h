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
#define ICON_SINGLE_SHOT        5
#define ICON_BURST_SHOT         6
#define ICON_FULL_AUTO          7
#define ICON_GREEN_AP           8
#define ICON_GRAY_AP            9
#define ICON_RED_AP             10
#define ICON_YELLOW_AP          11

#define ICON_CIRCLE_PLUS        16
#define ICON_SQUARE_FOOT        17
#define ICON_SQUARE             18
#define ICON_LEFT               19
#define ICON_RIGHT              20
#define ICON_DOWN               21
#define ICON_UP                 22

inline void DrawIcon(GameData& data, int x, int y, Color tint, int icon) {
    int sheetId = data.ui.iconSpriteSheet;
    Texture2D tex = data.spriteData.sheet.texture[sheetId];
    Rectangle srcRect = data.spriteData.sheet.frameRects[sheetId][icon];
    DrawTextureRec(tex, srcRect, { (float) x, (float) y}, tint);
}


#endif //SANDBOX_ICONS_H
