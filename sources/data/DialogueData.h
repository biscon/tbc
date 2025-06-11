//
// Created by bison on 10-06-25.
//

#ifndef SANDBOX_DIALOGUEDATA_H
#define SANDBOX_DIALOGUEDATA_H

#include <string>
#include <vector>
#include "util/json.hpp"

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
};

#endif //SANDBOX_DIALOGUEDATA_H
