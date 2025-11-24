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
#include "graphics/TileMap.h"
#include "graphics/Lighting.h"
#include "ScriptSystem.h"
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

void PushExitLevel(ActionQueue& q, const std::string& levelFile, const std::string& spawnPoint, const std::string& onEnterFunc) {
    q.push({ ActionType::ExitLevel, ExitLevelAction{levelFile, spawnPoint, onEnterFunc} });
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

void PushDoorInteract(ActionQueue& q, const std::string& doorId) {
    q.push({ ActionType::DoorInteract, DoorInteractAction{doorId} });
}

void PushSpeechBubble(ActionQueue &q, const std::string &text, const Vector2i &pos, float duration) {
    q.push({ ActionType::SpeechBubble, SpeechBubbleAction{pos, duration, text} });
}

// -----------------------------------------------------------------------------
// ACTION PROCESSING
// -----------------------------------------------------------------------------

static void ExecuteDoorInteractAction(GameData& data, Level& level, const DoorInteractAction& act)
{
    SpriteData& spriteData = data.spriteData;

    // Look up door
    auto it = level.doors.find(act.doorId);
    if (it == level.doors.end())
        return;

    auto& door = it->second;
    DoorSaveState& doorState = data.levelState[level.name].doors[door.id];

    // Toggle open → closed or closed → open
    if (!doorState.open)
    {
        TraceLog(LOG_INFO, "Opening door %s", door.id.c_str());
        doorState.open = true;

        SetReverseSpriteAnimation(spriteData, door.animPlayer, false);
        ResumeSpriteAnimation(spriteData, door.animPlayer);
        SetFrame(spriteData, door.animPlayer, 0);
    }
    else
    {
        TraceLog(LOG_INFO, "Closing door %s", door.id.c_str());
        doorState.open = false;

        SetReverseSpriteAnimation(spriteData, door.animPlayer, true);
        int anim = spriteData.player.animationIdx[door.animPlayer];
        int frames = (int)spriteData.anim.frames[anim].size();
        SetFrame(spriteData, door.animPlayer, frames - 1);
        ResumeSpriteAnimation(spriteData, door.animPlayer);
    }

    // Update tile layers
    SetTiles(level.tileMap, door.blockedTiles, NAV_LAYER, doorState.open ? 0 : 1);
    SetTiles(level.tileMap, door.shadowTiles, SHADOW_LAYER, doorState.open ? 0 : 1);
    SetTiles(level.tileMap, door.shadowTiles, LIGHT_LAYER, doorState.open ? 0 : 1);

    // Recompute lighting
    PropagateLight(level.lighting, level.tileMap);
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

                if(!ev.onEnterFunc.empty()) {
                    bool result = false;
                    ScriptSystemCallFunctionBool(data.scriptData, level.name, "Level." + ev.onEnterFunc, result);
                    if(!result)
                        return false;
                }

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

            case ActionType::DoorInteract: {
                ExecuteDoorInteractAction(
                        data,
                        level,
                        std::get<DoorInteractAction>(a.payload)
                );
                break;
            }

            case ActionType::SpeechBubble:
            {
                // Erase existing speech bubbles, probably need to be a param in the future
                level.animations.erase(
                        std::remove_if(level.animations.begin(), level.animations.end(),
                                       [](const Animation& anim) {
                                           return anim.type == AnimationType::SpeechBubble;
                                       }),
                        level.animations.end()
                );
                auto& ev = std::get<SpeechBubbleAction>(a.payload);
                Animation speechBubble{};
                SetupSpeechBubbleAnimation(speechBubble, ev.text.c_str(), ev.pos.x, ev.pos.y, ev.duration, 0.0f);
                level.animations.push_back(speechBubble);
                break;
            }

                // -----------------------------------------------------------------
            default:
                break;
        }
    }
    return false;
}

void ExecutePendingAction(GameData &data) {
    if(data.levelData.pendingAction.hasPending) {
        data.levelData.pendingAction.hasPending = false;
        TraceLog(LOG_DEBUG, "Executing pending action...");
        data.actionQueue.push(std::move(data.levelData.pendingAction.action));
    }
}

