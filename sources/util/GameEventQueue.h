//
// Created by bison on 20-02-25.
//

#ifndef SANDBOX_GAMEEVENTQUEUE_H
#define SANDBOX_GAMEEVENTQUEUE_H

#include <vector>
#include "MathUtil.h"

enum class GameEventType {
    MoveParty,
    PartySpotted,
    EndCombat,
    ExitLevel,
    InitiateDialogue,
    EndDialogue,
    StartQuest,
    OpenInventory,
    CloseInventory,
    OpenMenu,
};

struct MovePartyEvent {
    Vector2i target;
};

struct PartySpottedEvent {
    int spotter;
};

struct EndCombatEvent {
    bool victory;
};

struct ExitLevelEvent {
    char levelFile[64];
    char spawnPoint[64];
};

struct InitiateDialogueEvent {
    int npcId;
    int dialogueNodeId;
};

struct EndDialogueEvent {
    int npcId;
};

struct OpenInventoryEvent {
    int charId;
};

struct StartQuestEvent {
    char questId[128];
};

struct GameEvent {
    GameEventType type;
    union {
        MovePartyEvent moveParty;
        PartySpottedEvent partySpotted;
        EndCombatEvent endCombat;
        ExitLevelEvent exitLevelEvent;
        InitiateDialogueEvent initiateDialogueEvent;
        EndDialogueEvent endDialogueEvent;
        StartQuestEvent startQuestEvent;
        OpenInventoryEvent openInventoryEvent;
    };
};

struct GameEventQueue {
    std::vector<GameEvent> events;
};

void PublishEvent(GameEventQueue &eventQueue, GameEvent event);
bool GetNextEvent(GameEventQueue &eventQueue, GameEvent &event);

void PublishMovePartyEvent(GameEventQueue &eventQueue, Vector2i target);
void PublishPartySpottedEvent(GameEventQueue &eventQueue, int spotter);
void PublishEndCombatEvent(GameEventQueue &eventQueue, bool victory);
void PublishExitLevelEvent(GameEventQueue &eventQueue, const std::string& levelFile, const std::string& spawnPoint);
void PublishInitiateDialogueEvent(GameEventQueue &eventQueue, int npcId, int dialogueNodeId);
void PublishEndDialogueEvent(GameEventQueue &eventQueue, int npcId);
void PublishStartQuestEvent(GameEventQueue &eventQueue, const std::string& questId);
void PublishOpenInventoryEvent(GameEventQueue &eventQueue, int charId);
void PublishCloseInventoryEvent(GameEventQueue &eventQueue);
void PublishOpenMenuEvent(GameEventQueue &eventQueue);

#endif //SANDBOX_GAMEEVENTQUEUE_H
