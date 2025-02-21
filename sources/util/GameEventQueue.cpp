//
// Created by bison on 20-02-25.
//

#include "GameEventQueue.h"

void PublishEvent(GameEventQueue &eventQueue, GameEvent event) {
    eventQueue.events.push_back(event);
}

bool GetNextEvent(GameEventQueue &eventQueue, GameEvent &event) {
    if(eventQueue.events.empty()) {
        return false;
    }
    event = eventQueue.events.front();
    eventQueue.events.erase(eventQueue.events.begin());
    return true;
}

void PublishMovePartyEvent(GameEventQueue &eventQueue, Vector2i target) {
    GameEvent event{};
    event.type = GameEventType::MoveParty;
    event.moveParty.target = target;
    PublishEvent(eventQueue, event);
}
