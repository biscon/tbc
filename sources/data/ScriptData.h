//
// Created by bison on 05-11-25.
//

#ifndef SANDBOX_SCRIPTDATA_H
#define SANDBOX_SCRIPTDATA_H

#include <unordered_map>

extern "C" {
    #include "wren/wren.h"
}

struct ScriptData {
    WrenVM* vm = nullptr;
    std::unordered_map<std::string, bool> flags;
};


#endif //SANDBOX_SCRIPTDATA_H
