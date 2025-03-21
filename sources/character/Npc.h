//
// Created by bison on 11-03-25.
//

#ifndef SANDBOX_NPC_H
#define SANDBOX_NPC_H

#include <string>
#include <vector>
#include "data/GameData.h"

void InitNpcTemplateData(NpcTemplateData& data, const std::string& filename);
int CreateCharacterFromTemplate(GameData& data, const std::string &name);



#endif //SANDBOX_NPC_H
