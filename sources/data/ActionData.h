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
};

struct InitiateDialogueAction {
    int npcId;
    int dialogueNodeId;
};

struct EndDialogueAction {
    int npcId;
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
        OpenLootInventoryAction
>;

// -----------------------------------------------------------------------------
// GAME ACTION
// -----------------------------------------------------------------------------

struct GameAction {
    ActionType type;
    ActionPayload payload;
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
