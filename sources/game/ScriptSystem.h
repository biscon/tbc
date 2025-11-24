//
// Created by bison on 05-11-25.
//

#ifndef SANDBOX_SCRIPTSYSTEM_H
#define SANDBOX_SCRIPTSYSTEM_H

#include <string>
#include "data/ScriptData.h"
#include "data/GameData.h"

// Initialize once at game startup
void ScriptSystemInit(GameData& script, Level& currentLevel);

// Shutdown when quitting
void ScriptSystemShutdown(ScriptData& script);

std::string SanitizeModuleName(const std::string& input);

// Execute a Wren script file inside a module (e.g. "map_forest")
bool ScriptSystemRunFile(ScriptData& script, const std::string& moduleName, const std::string& filePath);

bool ScriptSystemCallFunction(ScriptData& script,
                              const std::string& moduleName,
                              const std::string& functionName);


bool ScriptSystemCallFunctionBool(ScriptData& script,
                                  const std::string& moduleName,
                                  const std::string& functionName,
                                  bool& outResult);

bool ScriptSystemCallFunctionInt(ScriptData& script,
                                 const std::string& moduleName,
                                 const std::string& functionName,
                                 int& outResult);

bool ScriptSystemCallFunctionDouble(ScriptData& script,
                                    const std::string& moduleName,
                                    const std::string& functionName,
                                    double& outResult);

bool ScriptSystemCallFunctionString(ScriptData& script,
                                    const std::string& moduleName,
                                    const std::string& functionName,
                                    std::string& outResult);


#endif //SANDBOX_SCRIPTSYSTEM_H
