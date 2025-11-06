//
// Created by bison on 05-11-25.
//

#include "ScriptSystem.h"
#include "data/GameData.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <cstring>

// -----------------------------------------------------------------------------
// Simple in-memory game flags for demo. Replace with your real game state.
static GameData* gameData = nullptr;

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
        // Replace this with real engine call
        std::cout << "[GAME] Weather set to: " << weather << "\n";
    }
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
        if (strcmp(signature, "setWeather(_)") == 0) return C_setWeather;
        if (strcmp(signature, "showText(_)") == 0)   return C_showText;
        if (strcmp(signature, "setFlag(_,_)") == 0)  return C_setFlag;
        if (strcmp(signature, "flag(_)") == 0)       return C_flag;
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
            foreign static showText(text)
            foreign static setFlag(name, value)
            foreign static flag(name)
        }
    )";

    // If interpretation fails here, that's a sign of misconfiguration.
    wrenInterpret(script.vm, "game", decls);
}

// -----------------------------------------------------------------------------
// Initialize & shutdown
void ScriptSystemInit(GameData& data) {
    gameData = &data;
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

bool ScriptSystemCallFunction(ScriptData& script,
                              const std::string& moduleName,
                              const std::string& functionName)
{
    if (!script.vm) return false;

    // Sanitize module name to avoid spaces/punctuation issues
    std::string safeModule = SanitizeModuleName(moduleName);

    size_t dot = functionName.find('.');
    if (dot == std::string::npos) {
        std::cerr << "[WREN] Internal error: malformed signature '" << functionName << "'\n";
        return false;
    }

    std::string sig = functionName.substr(dot+1);
    std::string className = functionName.substr(0, dot);

    WrenHandle* callHandle = wrenMakeCallHandle(script.vm, sig.c_str());
    if (!callHandle) {
        std::cerr << "[WREN] Failed to create call handle for '" << sig << "'\n";
        return false;
    }

    wrenEnsureSlots(script.vm, 1);
    wrenGetVariable(script.vm, safeModule.c_str(), className.c_str(), 0);

    WrenType type = wrenGetSlotType(script.vm, 0);
    if (type == WREN_TYPE_NULL) {
        std::cerr << "[WREN] Class '" << className
                  << "' not found in module '" << safeModule << "'\n";
        wrenReleaseHandle(script.vm, callHandle);
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
