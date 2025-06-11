//
// Created by bison on 11-06-25.
//

#include <fstream>
#include "Dialogue.h"
#include "raylib.h"

void InitDialogueData(DialogueData& data, const std::string &filename) {
    std::ifstream file(filename);
    if (!file) {
        TraceLog(LOG_ERROR, "No dialogue data found.");
        std::abort();
    }

    nlohmann::json j;
    file >> j;
    // Clear existing data
    data.dialogueNodes.clear();
    data.dialogueResponses.clear();

    // Handle dialogueNodes
    const nlohmann::json& nodes = j.at("dialogueNodes");
    for (nlohmann::json::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
        int id = std::stoi(it.key());
        data.dialogueNodes[id] = it.value().get<DialogueNode>();
    }

    // Handle dialogueResponses
    const nlohmann::json& responses = j.at("dialogueResponses");
    for (nlohmann::json::const_iterator it = responses.begin(); it != responses.end(); ++it) {
        int id = std::stoi(it.key());
        data.dialogueResponses[id] = it.value().get<DialogueResponse>();
    }
}