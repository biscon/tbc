//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_GAMEDATA_H
#define SANDBOX_GAMEDATA_H

#include "WeaponData.h"
#include "CharacterData.h"
#include "SpriteData.h"
#include "NpcTemplateData.h"
#include "AnimationData.h"
#include "SkillData.h"
#include "StatusEffectData.h"
#include "TileMapData.h"

enum class GameState {
    START_NEW_GAME,
    LOAD_LEVEL,
    PLAY_LEVEL
};

struct GameData {
    std::vector<int> party;
    std::string levelFileName;
    GameState state;
    WeaponData weaponData;
    CharacterData charData;
    SpriteData spriteData;
    NpcTemplateData npcTemplateData;
};

#endif //SANDBOX_GAMEDATA_H
