//
// Created by bison on 10-06-25.
//

#ifndef SANDBOX_QUESTDATA_H
#define SANDBOX_QUESTDATA_H

#include "util/json.hpp"

enum class QuestStatus { NotStarted, Active, Completed, Failed };

struct Quest {
    std::string id;
    std::string title;
    std::string description;
};

struct QuestSaveState {
    QuestStatus status;
    int stage; // optional: which part of the quest you're on
};

void to_json(nlohmann::json& j, const QuestSaveState& quest);
void from_json(const nlohmann::json& j, QuestSaveState& quest);

// Helpers for string conversion
std::string ToString(QuestStatus status);
QuestStatus QuestStatusFromString(const std::string& str);

struct QuestData {
    std::unordered_map<std::string, Quest> quests;
};

#endif //SANDBOX_QUESTDATA_H
