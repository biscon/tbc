//
// Created by bison on 10-06-25.
//

#include "QuestData.h"

void to_json(nlohmann::json& j, const QuestSaveState& quest) {
    std::string statusStr;
    switch (quest.status) {
        case QuestStatus::NotStarted: statusStr = "NotStarted"; break;
        case QuestStatus::Active:     statusStr = "Active";     break;
        case QuestStatus::Completed:  statusStr = "Completed";  break;
        case QuestStatus::Failed:  statusStr = "Failed";  break;
    }

    j = nlohmann::json{
            {"status", statusStr},
            {"stage", quest.stage}
    };
}

void from_json(const nlohmann::json& j, QuestSaveState& quest) {
    std::string statusStr;
    j.at("status").get_to(statusStr);

    if (statusStr == "NotStarted")      quest.status = QuestStatus::NotStarted;
    else if (statusStr == "Active")     quest.status = QuestStatus::Active;
    else if (statusStr == "Completed")  quest.status = QuestStatus::Completed;
    else if (statusStr == "Failed")  quest.status = QuestStatus::Failed;
    else throw std::invalid_argument("Unknown quest status: " + statusStr);

    j.at("stage").get_to(quest.stage);
}