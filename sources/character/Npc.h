//
// Created by bison on 11-03-25.
//

#ifndef SANDBOX_NPC_H
#define SANDBOX_NPC_H

#include <string>
#include <vector>
#include "data/GameData.h"
#include "util/MathUtil.h"

struct NpcInstance {
    std::string npcTemplate;
    Vector2i position;
    int dialogueNodeId;
};

// JSON serialization
void to_json(nlohmann::json& j, const NpcInstance& npc);
void from_json(const nlohmann::json& j, NpcInstance& npc);

void InitNpcTemplateData(NpcTemplateData& data, const std::string& filename);
int CreateCharacterFromTemplate(GameData& data, const std::string &npcTemplate);



#endif //SANDBOX_NPC_H
