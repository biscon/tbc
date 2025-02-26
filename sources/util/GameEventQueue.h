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
};

struct MovePartyEvent {
    Vector2i target;
};

struct PartySpottedEvent {
    Character *spotter;
};

struct EndCombatEvent {
    bool victory;
};

struct GameEvent {
    GameEventType type;
    union {
        MovePartyEvent moveParty;
        PartySpottedEvent partySpotted;
        EndCombatEvent endCombat;
    };
};

struct GameEventQueue {
    std::vector<GameEvent> events;
};

void PublishEvent(GameEventQueue &eventQueue, GameEvent event);
bool GetNextEvent(GameEventQueue &eventQueue, GameEvent &event);

void PublishMovePartyEvent(GameEventQueue &eventQueue, Vector2i target);
void PublishPartySpottedEvent(GameEventQueue &eventQueue, Character* spotter);
void PublishEndCombatEvent(GameEventQueue &eventQueue, bool victory);

#endif //SANDBOX_GAMEEVENTQUEUE_H
