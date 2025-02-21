//
// Created by bison on 20-02-25.
//

#ifndef SANDBOX_GAMEEVENTQUEUE_H
#define SANDBOX_GAMEEVENTQUEUE_H

#include <vector>
#include "MathUtil.h"

enum class GameEventType {
    MoveParty,
};

struct MovePartyEvent {
    Vector2i target;
};

struct GameEvent {
    GameEventType type;
    union {
        MovePartyEvent moveParty;
    };
};

struct GameEventQueue {
    std::vector<GameEvent> events;
};

void PublishEvent(GameEventQueue &eventQueue, GameEvent event);
bool GetNextEvent(GameEventQueue &eventQueue, GameEvent &event);

void PublishMovePartyEvent(GameEventQueue &eventQueue, Vector2i target);

#endif //SANDBOX_GAMEEVENTQUEUE_H
