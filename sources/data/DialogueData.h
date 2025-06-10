//
// Created by bison on 10-06-25.
//

#ifndef SANDBOX_DIALOGUEDATA_H
#define SANDBOX_DIALOGUEDATA_H

#include <string>
#include <vector>

enum class ConditionType { QuestStatusEquals, HasItem, FlagIsSet };
struct Condition { ConditionType type; std::string param; int value; };

enum class EffectType { StartQuest, CompleteQuest, SetFlag, GiveItem };
struct Effect { EffectType type; std::string param; int value; };

struct DialogueNode {
    int id;
    std::string text;
    std::vector<int> responseIds; // links to DialogueResponses
};

struct DialogueResponse {
    int id;
    std::string text;
    int nextNodeId;
    std::string startQuestId;      // optional quest to start
    std::string completeQuestId;   // optional quest to complete
};

#endif //SANDBOX_DIALOGUEDATA_H
