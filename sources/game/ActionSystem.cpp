//
// Created by bison on 19-11-25.
//

#include "ActionSystem.h"

#include "character/Character.h"
#include "level/Level.h"
#include "rcamera.h"
#include "level/CombatEngine.h"
#include "ui/Dialogue.h"
#include "graphics/Animation.h"
#include "level/LevelCamera.h"
#include "ui/ActionBar.h"
#include "ui/LootInventory.h"
#include "audio/Sound.h"
#include "game/Input.h"
#include "level/LevelSystem.h"
// -----------------------------------------------------------------------------
// QUEUE POP
// -----------------------------------------------------------------------------

bool ActionQueue::pop(GameAction& out) {
    if (actions.empty()) return false;
    out = std::move(actions.front());
    actions.erase(actions.begin());
    return true;
}

// -----------------------------------------------------------------------------
// PUBLISH HELPERS
// -----------------------------------------------------------------------------

void PushMoveParty(ActionQueue& q, Vector2i target) {
    q.push({ ActionType::MoveParty, MovePartyAction{target} });
}

void PushPartySpotted(ActionQueue& q, int spotter) {
    q.push({ ActionType::PartySpotted, PartySpottedAction{spotter} });
}

void PushEndCombat(ActionQueue& q, bool victory) {
    q.push({ ActionType::EndCombat, EndCombatAction{victory} });
}

void PushExitLevel(ActionQueue& q, const std::string& levelFile, const std::string& spawnPoint) {
    q.push({ ActionType::ExitLevel, ExitLevelAction{levelFile, spawnPoint} });
}

void PushInitiateDialogue(ActionQueue& q, int npcId, int dialogueNodeId) {
    q.push({ ActionType::InitiateDialogue, InitiateDialogueAction{npcId, dialogueNodeId} });
}

void PushEndDialogue(ActionQueue& q, int npcId) {
    q.push({ ActionType::EndDialogue, EndDialogueAction{npcId} });
}

void PushStartQuest(ActionQueue& q, const std::string& questId) {
    q.push({ ActionType::StartQuest, StartQuestAction{questId} });
}

void PushOpenInventory(ActionQueue& q, int charId) {
    q.push({ ActionType::OpenInventory, OpenInventoryAction{charId} });
}

void PushCloseInventory(ActionQueue& q) {
    q.push({ ActionType::CloseInventory, std::monostate{} });
}

void PushOpenMenu(ActionQueue& q) {
    q.push({ ActionType::OpenMenu, std::monostate{} });
}

void PushOpenActionBar(ActionQueue& q) {
    q.push({ ActionType::OpenActionBar, std::monostate{} });
}

void PushCloseActionBar(ActionQueue& q) {
    q.push({ ActionType::CloseActionBar, std::monostate{} });
}

void PushOpenLootInventory(ActionQueue& q, int invId) {
    q.push({ ActionType::OpenLootInventory, OpenLootInventoryAction{invId} });
}

void PushCloseLootInventory(ActionQueue& q) {
    q.push({ ActionType::CloseLootInventory, std::monostate{} });
}

// -----------------------------------------------------------------------------
// ACTION PROCESSING
// -----------------------------------------------------------------------------

void ProcessActionsOLD(GameData& data, Level& level, LevelSystemData& playField, ActionQueue& queue, float dt)
{
    GameAction a;
    while (queue.pop(a))
    {
        switch (a.type)
        {
            case ActionType::MoveParty: {
                auto& ev = std::get<MovePartyAction>(a.payload);
                // your logic here
                break;
            }

            case ActionType::PartySpotted: {
                auto& ev = std::get<PartySpottedAction>(a.payload);
                break;
            }

            case ActionType::EndCombat: {
                auto& ev = std::get<EndCombatAction>(a.payload);
                break;
            }

            case ActionType::ExitLevel: {
                auto& ev = std::get<ExitLevelAction>(a.payload);
                break;
            }

            case ActionType::InitiateDialogue: {
                auto& ev = std::get<InitiateDialogueAction>(a.payload);
                break;
            }

            case ActionType::EndDialogue: {
                auto& ev = std::get<EndDialogueAction>(a.payload);
                break;
            }

            case ActionType::StartQuest: {
                auto& ev = std::get<StartQuestAction>(a.payload);
                break;
            }

            case ActionType::OpenInventory: {
                auto& ev = std::get<OpenInventoryAction>(a.payload);
                break;
            }

            case ActionType::CloseInventory:
                break;

            case ActionType::OpenMenu:
                break;

            case ActionType::OpenActionBar:
                break;

            case ActionType::CloseActionBar:
                break;

            case ActionType::OpenLootInventory: {
                auto& ev = std::get<OpenLootInventoryAction>(a.payload);
                break;
            }

            case ActionType::CloseLootInventory:
                break;
        }
    }
}

// return true if the gamemode should be popped
bool ProcessActions(GameData& data, Level& level, float dt)
{
    LevelSystemData& levelData = data.levelData;
    GameAction a;
    while (data.actionQueue.pop(a))
    {
        switch (a.type)
        {
            // -----------------------------------------------------------------
            // MOVE PARTY
            // -----------------------------------------------------------------
            case ActionType::MoveParty:
            {
                auto& ev = std::get<MovePartyAction>(a.payload);

                StartCameraPanToTilePos(level.camera, ev.target, 250.0f);


                TraceLog(LOG_INFO, "MoveParty event,target: %d,%d", ev.target.x, ev.target.y);
                levelData.activeMoves.clear();
                MoveCharacter(data, level, data.ui.selectedCharacter, ev.target);
                // move the rest partially
                for(int i = 0; i < (int)level.partyCharacters.size(); i++) {
                    if(level.partyCharacters[i] != data.ui.selectedCharacter) {
                        MoveCharacterPartial(data, level, level.partyCharacters[i], ev.target);
                    }
                }

                if (level.footStepsHandle != -1) {
                    StopSfx(data.soundData, level.footStepsHandle);
                }
                level.footStepsHandle = PlaySfx(data.soundData, "footstep", true);

                break;
            }

                // -----------------------------------------------------------------
                // PARTY SPOTTED
                // -----------------------------------------------------------------
            case ActionType::PartySpotted:
            {
                auto& ev = std::get<PartySpottedAction>(a.payload);

                data.ui.inCombat = true;
                levelData.mode = LevelMode::None;
                StartCombat(data.spriteData, data.charData, level, ev.spotter);

                break;
            }

                // -----------------------------------------------------------------
                // END COMBAT
                // -----------------------------------------------------------------
            case ActionType::EndCombat:
            {
                auto& ev = std::get<EndCombatAction>(a.payload);

                level.turnState = TurnState::None;
                levelData.mode = LevelMode::Explore;
                data.ui.inCombat = false;

                for (auto& c : level.partyCharacters)
                {
                    CharacterSprite& sprite = data.charData.sprite[c];
                    StartPausedCharacterSpriteAnim(
                            data.spriteData,
                            sprite,
                            SpriteAnimationType::WalkDown,
                            true
                    );

                    data.charData.orientation[c] = Orientation::Down;
                    data.charData.statusEffects[c].clear();

                    if (data.charData.stats[c].HP <= 0) {
                        data.charData.stats[c].HP =
                                CalculateCharHealth(data.charData.stats[c]);
                        SetCharacterSpriteRotation(data.spriteData, sprite, 0);
                    }
                }

                break;
            }

                // -----------------------------------------------------------------
                // EXIT LEVEL
                // -----------------------------------------------------------------
            case ActionType::ExitLevel:
            {
                auto& ev = std::get<ExitLevelAction>(a.payload);

                TraceLog(LOG_INFO, "ExitLevel: %s, spawnPoint: %s",
                         ev.levelFile.c_str(),
                         ev.spawnPoint.c_str());

                data.levelFileName = ev.levelFile;

                ResetLevelSystem(levelData);
                LoadLevel(data, level, data.levelFileName);

                AddPartyToLevel(
                        data.spriteData,
                        data.charData,
                        level,
                        data.party,
                        ev.spawnPoint
                );

                StartCameraPanToTargetCharTime(
                        data.spriteData,
                        data.charData,
                        level.camera,
                        data.party[0],
                        0.01f
                );

                data.state = GameState::PLAY_LEVEL;
                levelData.mode = LevelMode::Explore;

                break;
            }

                // -----------------------------------------------------------------
                // INITIATE DIALOGUE
                // -----------------------------------------------------------------
            case ActionType::InitiateDialogue:
            {
                auto& ev = std::get<InitiateDialogueAction>(a.payload);

                levelData.mode = LevelMode::None;
                data.state = GameState::DIALOGUE;

                TraceLog(LOG_INFO,
                         "InitiateDialogue: npcId = %i, dialogueNodeId = %i",
                         ev.npcId, ev.dialogueNodeId
                );

                InitiateDialogue(data, ev.dialogueNodeId, ev.npcId);

                break;
            }

                // -----------------------------------------------------------------
                // END DIALOGUE
                // -----------------------------------------------------------------
            case ActionType::EndDialogue:
            {
                auto& ev = std::get<EndDialogueAction>(a.payload);

                TraceLog(LOG_INFO, "EndDialogue: npcId = %i", ev.npcId);

                levelData.mode = LevelMode::Explore;
                data.state = GameState::PLAY_LEVEL;

                break;
            }

                // -----------------------------------------------------------------
                // START QUEST
                // -----------------------------------------------------------------
            case ActionType::StartQuest:
            {
                auto& ev = std::get<StartQuestAction>(a.payload);

                const Quest& quest = data.questData.quests[ev.questId];

                Animation textAnim1{};
                Animation textAnim2{};

                SetupFancyTextAnimation(textAnim1,
                                        "Quest started:",
                                        10, 285,
                                        2.0f, 0.5f, 0.05f, 1.0f);

                SetupFancyTextAnimation(textAnim2,
                                        quest.title.c_str(),
                                        20, 300,
                                        2.0f, 1.0f, 0.05f, 1.0f);

                level.animations.push_back(textAnim1);
                level.animations.push_back(textAnim2);

                break;
            }

                // -----------------------------------------------------------------
                // OPEN INVENTORY
                // -----------------------------------------------------------------
            case ActionType::OpenInventory:
            {
                auto& ev = std::get<OpenInventoryAction>(a.payload);

                data.state = GameState::INVENTORY;
                data.ui.selectedCharacter = ev.charId;

                break;
            }

                // -----------------------------------------------------------------
                // CLOSE INVENTORY
                // -----------------------------------------------------------------
            case ActionType::CloseInventory:
            {
                data.state = GameState::PLAY_LEVEL;
                break;
            }

                // -----------------------------------------------------------------
                // OPEN MENU
                // -----------------------------------------------------------------
            case ActionType::OpenMenu:
            {
                return true;
            }

                // -----------------------------------------------------------------
                // OPEN ACTION BAR
                // -----------------------------------------------------------------
            case ActionType::OpenActionBar:
            {
                data.ui.showActionBar = true;
                data.ui.actionBar.selectedActionIdx = 0;
                data.ui.actionBar.selectedModeIdx = 0;

                ExecuteAction(
                        data,
                        ActionBarAction::Move,
                        level,
                        levelData,
                        true
                );

                break;
            }

                // -----------------------------------------------------------------
                // CLOSE ACTION BAR
                // -----------------------------------------------------------------
            case ActionType::CloseActionBar:
            {
                data.ui.showActionBar = false;
                break;
            }

                // -----------------------------------------------------------------
                // OPEN LOOT INVENTORY
                // -----------------------------------------------------------------
            case ActionType::OpenLootInventory:
            {
                auto& ev = std::get<OpenLootInventoryAction>(a.payload);

                data.state = GameState::LOOT_INVENTORY;
                data.ui.lootInventory.inventoryId = ev.invId;
                InitLootInventory(data);

                break;
            }

                // -----------------------------------------------------------------
                // CLOSE LOOT INVENTORY
                // -----------------------------------------------------------------
            case ActionType::CloseLootInventory:
            {
                data.state = GameState::PLAY_LEVEL;
                break;
            }

                // -----------------------------------------------------------------
            default:
                break;
        }
    }
    return false;
}
