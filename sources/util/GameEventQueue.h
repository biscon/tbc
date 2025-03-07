//
// Created by bison on 20-02-25.
//

#ifndef SANDBOX_GAMEEVENTQUEUE_H
#define SANDBOX_GAMEEVENTQUEUE_H

#include <vector>
#include "MathUtil.h"
#include "character/Character.h"

enum class GameEventType {
    MoveParty,
    PartySpotted,
    EndCombat,
    ExitLevel,
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

struct GameEvent {
    GameEventType type;
    union {
        MovePartyEvent moveParty;
        PartySpottedEvent partySpotted;
        EndCombatEvent endCombat;
        ExitLevelEvent exitLevelEvent;
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

#endif //SANDBOX_GAMEEVENTQUEUE_H
