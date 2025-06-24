//
// Created by bison on 10-06-25.
//

#include "QuestData.h"

using nlohmann::json;

std::string ToString(QuestStatus status) {
    switch (status) {
        case QuestStatus::NotStarted: return "NotStarted";
        case QuestStatus::Active:     return "Active";
        case QuestStatus::Completed:  return "Completed";
        case QuestStatus::Failed:     return "Failed";
        default:                      return "Unknown";
    }
}

QuestStatus QuestStatusFromString(const std::string& str) {
    if (str == "NotStarted") return QuestStatus::NotStarted;
    if (str == "Active")     return QuestStatus::Active;
    if (str == "Completed")  return QuestStatus::Completed;
    if (str == "Failed")     return QuestStatus::Failed;
    throw std::invalid_argument("Unknown QuestStatus: " + str);
}

void to_json(json& j, const QuestSaveState& quest) {
    j = json{
            {"status", ToString(quest.status)},
            {"stage", quest.stage}
    };
}

void from_json(const json& j, QuestSaveState& quest) {
    std::string statusStr;
    j.at("status").get_to(statusStr);
    quest.status = QuestStatusFromString(statusStr);

    j.at("stage").get_to(quest.stage);
}
