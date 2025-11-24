//
// Created by bison on 05-11-25.
//

#include "ScriptSystem.h"
#include "data/GameData.h"
#include "level/Weather.h"
#include "ActionSystem.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <cstring>

// -----------------------------------------------------------------------------
// Simple in-memory game flags for demo. Replace with your real game state.
static GameData* gameData = nullptr;
static Level* level = nullptr;

// -----------------------------------------------------------------------------
// Wren output / error callbacks
static void WriteFn(WrenVM*, const char* text) {
    std::cout << "[WREN] " << text;
}

static void ErrorFn(WrenVM*, WrenErrorType type, const char* module, int line, const char* msg) {
    switch (type) {
        case WREN_ERROR_COMPILE:
            std::cerr << "[WREN:COMPILE] " << (module ? module : "<unknown>") << ":" << line
                      << " - " << msg << "\n";
            break;
        case WREN_ERROR_RUNTIME:
            std::cerr << "[WREN:RUNTIME] " << msg << "\n";
            break;
        case WREN_ERROR_STACK_TRACE:
            std::cerr << "[WREN:STACK] " << (module ? module : "<unknown>") << ":" << line
                      << " - " << msg << "\n";
            break;
    }
}

// -----------------------------------------------------------------------------
// Native functions exposed to Wren (WrenForeignMethodFn signature: void(*)(WrenVM*))
static void C_setWeather(WrenVM* vm) {
    const char* weather = wrenGetSlotString(vm, 1);
    if (weather) {
        if(strcmp(weather, "sunny") == 0) SetWeatherType(*gameData, WeatherType::Sunny, level->outdoor);
        if(strcmp(weather, "overcast") == 0) SetWeatherType(*gameData, WeatherType::Overcast, level->outdoor);
        if(strcmp(weather, "rain") == 0) SetWeatherType(*gameData, WeatherType::Rain, level->outdoor);
        if(strcmp(weather, "thunder") == 0) SetWeatherType(*gameData, WeatherType::Thunder, level->outdoor);
        std::cout << "[GAME] Weather set to: " << weather << "\n";
    }
}

static void C_setHourOfDay(WrenVM* vm) {
    auto hod = (float) wrenGetSlotDouble(vm, 1);
    if(hod >= 0 && hod <= 24) {
        std::cout << "[GAME] Hour of day set to: " << hod << "\n";
        level->hourOfDay = hod;
    } else {
        std::cerr << "[GAME] setHourOfDay argument must be between 0-24\n";
    }
}

static void C_getHourOfDay(WrenVM* vm) {
    wrenSetSlotDouble(vm, 0, level->hourOfDay);
}

static void C_showText(WrenVM* vm) {
    const char* text = wrenGetSlotString(vm, 1);
    if (text) {
        // Replace with UI system call in your engine
        std::cout << "[TEXT] " << text << "\n";
    }
}

static void C_setFlag(WrenVM* vm) {
    const char* name = wrenGetSlotString(vm, 1);
    bool value = wrenGetSlotBool(vm, 2);
    if (name) {
        gameData->scriptData.flags[std::string(name)] = value;
        // Optionally print for debug
        std::cout << "[GAME] setFlag '" << name << "' = " << (value ? "true" : "false") << "\n";
    }
}

static void C_flag(WrenVM* vm) {
    const char* name = wrenGetSlotString(vm, 1);
    bool val = false;
    auto& flags = gameData->scriptData.flags;
    if (name) {
        auto it = flags.find(name);
        if (it != flags.end()) val = it->second;
    }
    wrenSetSlotBool(vm, 0, val); // return boolean in slot 0
}

static void C_setInt(WrenVM* vm) {
    const char* name = wrenGetSlotString(vm, 1);
    int value = static_cast<int>(wrenGetSlotDouble(vm, 2));
    if (name) {
        gameData->scriptData.ints[std::string(name)] = value;
        // Optionally print for debug
        std::cout << "[GAME] setInt '" << name << "' = " << value << "\n";
    }
}

static void C_getInt(WrenVM* vm) {
    const char* name = wrenGetSlotString(vm, 1);
    int val = -1;
    auto& ints = gameData->scriptData.ints;
    if (name) {
        auto it = ints.find(name);
        if (it != ints.end()) val = it->second;
    }
    wrenSetSlotDouble(vm, 0, static_cast<double>(val)); // return boolean in slot 0
}

static void C_setString(WrenVM* vm) {
    const char* name = wrenGetSlotString(vm, 1);
    std::string value = wrenGetSlotString(vm, 2);
    if (name) {
        gameData->scriptData.strings[std::string(name)] = value;
        // Optionally print for debug
        std::cout << "[GAME] setString '" << name << "' = " << value << "\n";
    }
}

static void C_getString(WrenVM* vm) {
    const char* name = wrenGetSlotString(vm, 1);
    std::string val;
    auto& strings = gameData->scriptData.strings;
    if (name) {
        auto it = strings.find(name);
        if (it != strings.end()) val = it->second;
    }
    wrenSetSlotString(vm, 0, val.c_str()); // return boolean in slot 0
}


static void C_speechBubble(WrenVM* vm) {
    const char* text = wrenGetSlotString(vm, 1);
    int x = static_cast<int>(wrenGetSlotDouble(vm, 2));
    int y = static_cast<int>(wrenGetSlotDouble(vm, 3));
    float duration = static_cast<float>(wrenGetSlotDouble(vm, 4));
    PushSpeechBubble(gameData->actionQueue, text, {x, y}, duration);
}

// -----------------------------------------------------------------------------
// bindForeignMethod callback to map Wren signatures to native functions
static WrenForeignMethodFn BindForeignMethod(WrenVM* vm,
                                             const char* module,
                                             const char* className,
                                             bool isStatic,
                                             const char* signature)
{
    (void)vm; (void)isStatic; // unused here

    if (module == nullptr || className == nullptr || signature == nullptr) return nullptr;

    // We expose a single foreign class: Game
    if (strcmp(module, "game") == 0 && strcmp(className, "Game") == 0) {
        if (strcmp(signature, "setWeather(_)") == 0)                return C_setWeather;
        if (strcmp(signature, "setHourOfDay(_)") == 0)              return C_setHourOfDay;
        if (strcmp(signature, "getHourOfDay()") == 0)               return C_getHourOfDay;
        if (strcmp(signature, "showText(_)") == 0)                  return C_showText;
        if (strcmp(signature, "setFlag(_,_)") == 0)                 return C_setFlag;
        if (strcmp(signature, "flag(_)") == 0)                      return C_flag;
        if (strcmp(signature, "setInt(_,_)") == 0)                  return C_setInt;
        if (strcmp(signature, "getInt(_)") == 0)                    return C_getInt;
        if (strcmp(signature, "setString(_,_)") == 0)               return C_setString;
        if (strcmp(signature, "getString(_)") == 0)                 return C_getString;
        if (strcmp(signature, "speechBubble(_,_,_,_)") == 0)        return C_speechBubble;
    }

    return nullptr;
}

// -----------------------------------------------------------------------------
// Internal: register the 'Game' foreign class declarations (run once per VM)
static void ScriptSystemRegisterAPI_Internal(ScriptData& script) {
    // Declare the foreign class and its signatures to Wren so scripts can call them.
    // We only need to interpret these declarations once (per VM).
    const char* decls = R"(
        foreign class Game {
            foreign static setWeather(weather)
            foreign static setHourOfDay(hour)
            foreign static getHourOfDay()
            foreign static showText(text)
            foreign static setFlag(name, value)
            foreign static flag(name)
            foreign static setInt(name, value)
            foreign static getInt(name)
            foreign static setString(name, value)
            foreign static getString(name)
            foreign static speechBubble(text, x, y, duration)
        }
    )";

    // If interpretation fails here, that's a sign of misconfiguration.
    wrenInterpret(script.vm, "game", decls);
}

// -----------------------------------------------------------------------------
// Initialize & shutdown
void ScriptSystemInit(GameData& data, Level& currentLevel) {
    gameData = &data;
    level = &currentLevel;
    WrenConfiguration config;
    wrenInitConfiguration(&config);

    config.writeFn = WriteFn;
    config.errorFn = ErrorFn;

    // Bind foreign methods via our BindForeignMethod callback
    config.bindForeignMethodFn = BindForeignMethod;
    //config.bindForeignClassFn = nullptr; // not used for static/normal classes here

    // Create the VM and register API (declarations)
    data.scriptData.vm = wrenNewVM(&config);

    // Register the Game foreign-class declarations once for this VM
    ScriptSystemRegisterAPI_Internal(data.scriptData);
}

void ScriptSystemShutdown(ScriptData& script) {
    if (script.vm) {
        wrenFreeVM(script.vm);
        script.vm = nullptr;
    }
}

// -----------------------------------------------------------------------------


// Run a Wren script from file in the given module namespace
bool ScriptSystemRunFile(ScriptData& script, const std::string& moduleName, const std::string& filePath) {
    if (!script.vm) return false;

    // Sanitize module name to avoid spaces/punctuation issues
    std::string safeModule = SanitizeModuleName(moduleName);

    std::ifstream in(filePath);
    if (!in.is_open()) {
        std::cerr << "[WREN] Failed to open script file: " << filePath << "\n";
        return false;
    }

    std::stringstream buf;
    buf << in.rdbuf();
    std::string source = buf.str();

    WrenInterpretResult res = wrenInterpret(script.vm, safeModule.c_str(), source.c_str());
    if (res != WREN_RESULT_SUCCESS) {
        std::cerr << "[WREN] Script execution failed for module '" << safeModule
                  << "' file '" << filePath << "'\n";
        return false;
    }

    return true;
}

std::string SanitizeModuleName(const std::string& input)
{
    std::string out;
    out.reserve(input.size());
    for (unsigned char c : input)
    {
        if (std::isalnum(c))
            out.push_back(static_cast<char>(std::tolower(c)));
        else
            out.push_back('_');
    }
    return out;
}

static bool ScriptSystemCallFunctionInternal(
        ScriptData& script,
        const std::string& moduleName,
        const std::string& functionName,
        WrenHandle*& outHandle,
        std::string& outSig,
        std::string& outClassName,
        std::string& outSafeModule)
{
    if (!script.vm) return false;

    // Sanitize module name (unchanged)
    outSafeModule = SanitizeModuleName(moduleName);

    // Extract class + signature
    size_t dot = functionName.find('.');
    if (dot == std::string::npos) {
        std::cerr << "[WREN] Internal error: malformed signature '" << functionName << "'\n";
        return false;
    }

    outSig = functionName.substr(dot + 1);
    outClassName = functionName.substr(0, dot);

    // Create call handle
    outHandle = wrenMakeCallHandle(script.vm, outSig.c_str());
    if (!outHandle) {
        std::cerr << "[WREN] Failed to create call handle for '" << outSig << "'\n";
        return false;
    }

    // Prepare slot 0 with the class
    wrenEnsureSlots(script.vm, 1);
    wrenGetVariable(script.vm, outSafeModule.c_str(), outClassName.c_str(), 0);

    WrenType type = wrenGetSlotType(script.vm, 0);
    if (type == WREN_TYPE_NULL) {
        std::cerr << "[WREN] Class '" << outClassName
                  << "' not found in module '" << outSafeModule << "'\n";
        wrenReleaseHandle(script.vm, outHandle);
        outHandle = nullptr;
        return false;
    }

    return true;
}

bool ScriptSystemCallFunction(ScriptData& script,
                              const std::string& moduleName,
                              const std::string& functionName)
{
    WrenHandle* callHandle = nullptr;
    std::string sig, className, safeModule;

    if (!ScriptSystemCallFunctionInternal(
            script, moduleName, functionName,
            callHandle, sig, className, safeModule))
    {
        return false;
    }

    WrenInterpretResult result = wrenCall(script.vm, callHandle);
    wrenReleaseHandle(script.vm, callHandle);

    if (result != WREN_RESULT_SUCCESS) {
        std::cerr << "[WREN] Call to " << sig
                  << " in module '" << safeModule
                  << "' failed with code " << result << "\n";
        return false;
    }

    return true;
}

bool ScriptSystemCallFunctionBool(ScriptData& script,
                                  const std::string& moduleName,
                                  const std::string& functionName,
                                  bool& outResult)
{
    WrenHandle* callHandle = nullptr;
    std::string sig, className, safeModule;

    if (!ScriptSystemCallFunctionInternal(
            script, moduleName, functionName,
            callHandle, sig, className, safeModule))
    {
        return false;
    }

    WrenInterpretResult result = wrenCall(script.vm, callHandle);
    wrenReleaseHandle(script.vm, callHandle);

    if (result != WREN_RESULT_SUCCESS) {
        std::cerr << "[WREN] Call to " << sig
                  << " in module '" << safeModule
                  << "' failed with code " << result << "\n";
        return false;
    }

    // Extract bool return value from slot 0
    WrenType type = wrenGetSlotType(script.vm, 0);
    if (type != WREN_TYPE_BOOL) {
        std::cerr << "[WREN] Expected bool return from '" << sig
                  << "' but got type " << type << "\n";
        return false;
    }

    outResult = wrenGetSlotBool(script.vm, 0);
    return true;
}

bool ScriptSystemCallFunctionInt(ScriptData& script,
                                 const std::string& moduleName,
                                 const std::string& functionName,
                                 int& outResult)
{
    WrenHandle* callHandle = nullptr;
    std::string sig, className, safeModule;

    if (!ScriptSystemCallFunctionInternal(
            script, moduleName, functionName,
            callHandle, sig, className, safeModule))
    {
        return false;
    }

    WrenInterpretResult result = wrenCall(script.vm, callHandle);
    wrenReleaseHandle(script.vm, callHandle);

    if (result != WREN_RESULT_SUCCESS) {
        std::cerr << "[WREN] Call to " << sig
                  << " in module '" << safeModule
                  << "' failed with code " << result << "\n";
        return false;
    }

    // Extract numeric return value from slot 0
    WrenType type = wrenGetSlotType(script.vm, 0);
    if (type != WREN_TYPE_NUM) {
        std::cerr << "[WREN] Expected numeric return from '" << sig
                  << "' but got type " << type << "\n";
        return false;
    }

    double numValue = wrenGetSlotDouble(script.vm, 0);
    outResult = static_cast<int>(numValue);  // Wren numbers are doubles

    return true;
}

bool ScriptSystemCallFunctionDouble(ScriptData& script,
                                    const std::string& moduleName,
                                    const std::string& functionName,
                                    double& outResult)
{
    WrenHandle* callHandle = nullptr;
    std::string sig, className, safeModule;

    if (!ScriptSystemCallFunctionInternal(
            script, moduleName, functionName,
            callHandle, sig, className, safeModule))
    {
        return false;
    }

    WrenInterpretResult result = wrenCall(script.vm, callHandle);
    wrenReleaseHandle(script.vm, callHandle);

    if (result != WREN_RESULT_SUCCESS) {
        std::cerr << "[WREN] Call to " << sig
                  << " in module '" << safeModule
                  << "' failed with code " << result << "\n";
        return false;
    }

    // Return type must be a number
    WrenType type = wrenGetSlotType(script.vm, 0);
    if (type != WREN_TYPE_NUM) {
        std::cerr << "[WREN] Expected numeric return from '" << sig
                  << "' but got type " << type << "\n";
        return false;
    }

    outResult = wrenGetSlotDouble(script.vm, 0);
    return true;
}

bool ScriptSystemCallFunctionString(ScriptData& script,
                                    const std::string& moduleName,
                                    const std::string& functionName,
                                    std::string& outResult)
{
    WrenHandle* callHandle = nullptr;
    std::string sig, className, safeModule;

    if (!ScriptSystemCallFunctionInternal(
            script, moduleName, functionName,
            callHandle, sig, className, safeModule))
    {
        return false;
    }

    WrenInterpretResult result = wrenCall(script.vm, callHandle);
    wrenReleaseHandle(script.vm, callHandle);

    if (result != WREN_RESULT_SUCCESS) {
        std::cerr << "[WREN] Call to " << sig
                  << " in module '" << safeModule
                  << "' failed with code " << result << "\n";
        return false;
    }

    // Return type must be a string
    WrenType type = wrenGetSlotType(script.vm, 0);
    if (type != WREN_TYPE_STRING) {
        std::cerr << "[WREN] Expected string return from '" << sig
                  << "' but got type " << type << "\n";
        return false;
    }

    const char* wrenStr = wrenGetSlotString(script.vm, 0);
    outResult = (wrenStr != nullptr ? wrenStr : "");

    return true;
}

