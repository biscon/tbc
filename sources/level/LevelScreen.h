//
// Created by bison on 10-01-25.
//

#ifndef SANDBOX_LEVELSCREEN_H
#define SANDBOX_LEVELSCREEN_H

#include "PlayField.h"
#include "graphics/ParticleSystem.h"
#include "util/GameEventQueue.h"

struct ActionIcon {
    char text[64];
    char description[256];
    bool disabled;
    Skill* skill;
};

struct LevelScreen {
    int actionIconScrollIndex;
    bool showActionBarTitle;
    GameEventQueue* eventQueue;
    int floatingStatsCharacter;
};

void CreateLevelScreen(LevelScreen &levelScreen, GameEventQueue* eventQueue);
void DestroyLevelScreen(LevelScreen &levelScreen);
void DrawLevelScreen(GameData& data, Level &level, LevelScreen &levelScreen, PlayField &playField);
void UpdateLevelScreen(SpriteData& spriteData, CharacterData& charData, Level &level, LevelScreen &levelScreen, float dt);
void HandleInputLevelScreen(SpriteData& spriteData, CharacterData& charData, LevelScreen &levelScreen, Level &level);
//Character* SelectTargetBasedOnThreat(Level& level);


#endif //SANDBOX_LEVELSCREEN_H
