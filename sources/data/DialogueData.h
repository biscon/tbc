//
// Created by bison on 10-06-25.
//

#ifndef SANDBOX_DIALOGUEDATA_H
#define SANDBOX_DIALOGUEDATA_H

#include <string>
#include <vector>
#include "util/json.hpp"
#include "raylib.h"

enum class ConditionType { QuestStatusEquals, HasItem, FlagIsSet };
struct Condition { ConditionType type; std::string param; int value; };

enum class EffectType { StartQuest, CompleteQuest, SetFlag, GiveItem };
struct Effect { EffectType type; std::string param; int value; };

struct DialogueNode {
    int id;
    std::string text;
    std::vector<int> responseIds; // links to DialogueResponses
};

void to_json(nlohmann::json& j, const DialogueNode& node);
void from_json(const nlohmann::json& j, DialogueNode& node);

struct DialogueResponse {
    int id;
    std::string text;
    int nextNodeId;
    std::string startQuestId;      // optional quest to start
    std::string completeQuestId;   // optional quest to complete
};

void to_json(nlohmann::json& j, const DialogueResponse& resp);
void from_json(const nlohmann::json& j, DialogueResponse& resp);

struct DialogueData {
    std::unordered_map<int, DialogueNode> dialogueNodes;
    std::unordered_map<int, DialogueResponse> dialogueResponses;
    int currentDialogueNode = -1;
    int currentNpc = -1;
    std::vector<Rectangle> responseRects;
    float dialogueFade = 0.0f; // 0 = fully transparent, 1 = full opacity
    bool fadingOut = false;
    int idleAnimPlayer = -1;
    int talkAnimPlayer = -1;
};

#endif //SANDBOX_DIALOGUEDATA_H
