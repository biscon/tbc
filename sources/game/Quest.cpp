//
// Created by bison on 11-06-25.
//

#include <fstream>
#include "Quest.h"

void InitQuestData(GameData& data, const std::string &filename) {
    std::ifstream file(filename);
    if (!file) {
        TraceLog(LOG_ERROR, "No quest data found.");
        std::abort();
    }

    nlohmann::json j;
    file >> j;
    // Clear existing data
    data.quests.clear();

    // Handle QuestSaveState
    const nlohmann::json& nodes = j.at("quests");
    for (nlohmann::json::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
        const std::string& questId = it.key();
        data.quests[questId] = it.value().get<QuestSaveState>();
    }
}