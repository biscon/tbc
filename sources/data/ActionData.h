//
// Created by bison on 19-11-25.
//

#ifndef SANDBOX_ACTIONDATA_H
#define SANDBOX_ACTIONDATA_H

// -----------------------------------------------------------------------------
// ACTION TYPES
// -----------------------------------------------------------------------------

#include <variant>
#include <vector>
#include <string>
#include "util/MathUtil.h"

enum class ActionType {
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
    OpenActionBar,
    CloseActionBar,
    OpenLootInventory,
    CloseLootInventory,
    DoorInteract,
    SpeechBubble,
    ActivateTrigger
};

// -----------------------------------------------------------------------------
// ACTION PAYLOAD STRUCTS
// -----------------------------------------------------------------------------

struct MovePartyAction {
    Vector2i target;
};

struct PartySpottedAction {
    int spotter;
};

struct EndCombatAction {
    bool victory;
};

struct ExitLevelAction {
    std::string levelFile;
    std::string spawnPoint;
    std::string onEnterFunc;
};

struct InitiateDialogueAction {
    int dialogueNodeId;
};

struct EndDialogueAction {

};

struct OpenInventoryAction {
    int charId;
};

struct OpenLootInventoryAction {
    int invId;
};

struct StartQuestAction {
    std::string questId;
};

struct DoorInteractAction {
    std::string doorId;
};

struct SpeechBubbleAction {
    Vector2i pos;
    float duration;
    std::string text;
};

struct ActivateTriggerAction {
    std::string triggerId;
};

// -----------------------------------------------------------------------------
// ACTION UNION USING std::variant
// -----------------------------------------------------------------------------

using ActionPayload = std::variant<
        std::monostate,
        MovePartyAction,
        PartySpottedAction,
        EndCombatAction,
        ExitLevelAction,
        InitiateDialogueAction,
        EndDialogueAction,
        StartQuestAction,
        OpenInventoryAction,
        OpenLootInventoryAction,
        DoorInteractAction,
        SpeechBubbleAction,
        ActivateTriggerAction
>;

// -----------------------------------------------------------------------------
// GAME ACTION
// -----------------------------------------------------------------------------

struct GameAction {
    ActionType type;
    ActionPayload payload;
};

struct PendingAction {
    bool hasPending = false;
    GameAction action;
};

// -----------------------------------------------------------------------------
// ACTION QUEUE
// -----------------------------------------------------------------------------

struct ActionQueue {
    std::vector<GameAction> actions;
    void push(GameAction&& a) { actions.push_back(std::move(a)); }
    bool pop(GameAction& out);
};

#endif //SANDBOX_ACTIONDATA_H
