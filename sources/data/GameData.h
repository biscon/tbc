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
#include "SaveData.h"

enum class GameState {
    START_NEW_GAME,
    LOAD_LEVEL,
    LOAD_LEVEL_FROM_SAVE,
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
    std::unordered_map<std::string, LevelSaveState> levelState;
};

#endif //SANDBOX_GAMEDATA_H
