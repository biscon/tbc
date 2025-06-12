//
// Created by bison on 20-02-25.
//

#include <cstring>
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

void PublishPartySpottedEvent(GameEventQueue &eventQueue, int spotter) {
    GameEvent event{};
    event.type = GameEventType::PartySpotted;
    event.partySpotted.spotter = spotter;
    PublishEvent(eventQueue, event);
}

void PublishEndCombatEvent(GameEventQueue &eventQueue, bool victory) {
    GameEvent event{};
    event.type = GameEventType::EndCombat;
    event.endCombat.victory = victory;
    PublishEvent(eventQueue, event);
}

void PublishExitLevelEvent(GameEventQueue &eventQueue, const std::string &levelFile, const std::string& spawnPoint) {
    GameEvent event{};
    event.type = GameEventType::ExitLevel;
    strncpy(event.exitLevelEvent.levelFile, levelFile.c_str(), sizeof(event.exitLevelEvent.levelFile));
    strncpy(event.exitLevelEvent.spawnPoint, spawnPoint.c_str(), sizeof(event.exitLevelEvent.spawnPoint));
    PublishEvent(eventQueue, event);
}

void PublishInitiateDialogueEvent(GameEventQueue &eventQueue, int npcId, int dialogueNodeId) {
    GameEvent event{};
    event.type = GameEventType::InitiateDialogue;
    event.initiateDialogueEvent.npcId = npcId;
    event.initiateDialogueEvent.dialogueNodeId = dialogueNodeId;
    PublishEvent(eventQueue, event);
}

void PublishEndDialogueEvent(GameEventQueue &eventQueue, int npcId) {
    GameEvent event{};
    event.type = GameEventType::EndDialogue;
    event.endDialogueEvent.npcId = npcId;
    PublishEvent(eventQueue, event);
}
