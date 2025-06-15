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
#include "DialogueData.h"
#include "QuestData.h"
#include "SettingsData.h"

enum class GameState {
    START_NEW_GAME,
    LOAD_LEVEL,
    LOAD_LEVEL_FROM_SAVE,
    PLAY_LEVEL,
    DIALOGUE,
};

struct GameData {
    std::vector<int> party;
    std::string levelFileName;
    GameState state;
    WeaponData weaponData;
    CharacterData charData;
    SpriteData spriteData;
    NpcTemplateData npcTemplateData;
    DialogueData dialogueData;
    SettingsData settingsData;

    std::unordered_map<std::string, LevelSaveState> levelState;
    std::unordered_map<std::string, QuestSaveState> quests;
};

#endif //SANDBOX_GAMEDATA_H
