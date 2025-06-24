//
// Created by bison on 10-06-25.
//

#include "DialogueData.h"

using nlohmann::json;

// Helper for enum ↔ string mapping
static std::string ToString(ConditionType type) {
    switch (type) {
        case ConditionType::QuestStatusEquals: return "QuestStatusEquals";
        case ConditionType::HasItem: return "HasItem";
        case ConditionType::FlagIsSet: return "FlagIsSet";
        case ConditionType::GroupDefeated: return "GroupDefeated";
        default: return "Unknown";
    }
}

static ConditionType ConditionTypeFromString(const std::string& s) {
    if (s == "QuestStatusEquals") return ConditionType::QuestStatusEquals;
    if (s == "HasItem") return ConditionType::HasItem;
    if (s == "FlagIsSet") return ConditionType::FlagIsSet;
    if (s == "GroupDefeated") return ConditionType::GroupDefeated;
    throw std::invalid_argument("Invalid ConditionType: " + s);
}

std::string ToString(EffectType type) {
    switch (type) {
        case EffectType::StartQuest: return "StartQuest";
        case EffectType::CompleteQuest: return "CompleteQuest";
        case EffectType::SetFlag: return "SetFlag";
        case EffectType::GiveItem: return "GiveItem";
        default: return "Unknown";
    }
}

EffectType EffectTypeFromString(const std::string& s) {
    if (s == "StartQuest") return EffectType::StartQuest;
    if (s == "CompleteQuest") return EffectType::CompleteQuest;
    if (s == "SetFlag") return EffectType::SetFlag;
    if (s == "GiveItem") return EffectType::GiveItem;
    throw std::invalid_argument("Invalid EffectType: " + s);
}

// Condition JSON
void to_json(json& j, const Condition& c) {
    j = json{
            {"type", ToString(c.type)},
            {"param", c.param},
            {"value", c.value}
    };
}

void from_json(const json& j, Condition& c) {
    c.type = ConditionTypeFromString(j.at("type").get<std::string>());
    c.param = j.at("param").get<std::string>();
    c.value = j.at("value").get<std::string>();
}

// Effect JSON
void to_json(json& j, const Effect& e) {
    j = json{
            {"type", ToString(e.type)},
            {"param", e.param},
            {"value", e.value}
    };
}

void from_json(const json& j, Effect& e) {
    e.type = EffectTypeFromString(j.at("type").get<std::string>());
    e.param = j.at("param").get<std::string>();
    e.value = j.at("value").get<std::string>();
}

void to_json(nlohmann::json& j, const DialogueNode& node) {
    j = nlohmann::json {
            {"id", node.id},
            {"text", node.text},
            {"responseIds", node.responseIds},
            {"conditions", node.conditions},
            {"effects", node.effects},
            {"conditionalNextNodes", node.conditionalNextNodes}
    };
}

void from_json(const nlohmann::json& j, DialogueNode& node) {
    j.at("id").get_to(node.id);
    j.at("text").get_to(node.text);
    if(j.contains("responseIds")) {
        j.at("responseIds").get_to(node.responseIds);
    }
    if(j.contains("conditions")) {
        j.at("conditions").get_to(node.conditions);
    }
    if(j.contains("effects")) {
        j.at("effects").get_to(node.effects);
    }
    if(j.contains("conditionalNextNodes")) {
        j.at("conditionalNextNodes").get_to(node.conditionalNextNodes);
    }
}

void to_json(nlohmann::json& j, const DialogueResponse& resp) {
    j = nlohmann::json{
            {"id", resp.id},
            {"text", resp.text},
            {"nextNodeId", resp.nextNodeId},
            {"effects", resp.effects},
            {"conditions", resp.conditions},
    };
}

void from_json(const nlohmann::json& j, DialogueResponse& resp) {
    j.at("id").get_to(resp.id);
    j.at("text").get_to(resp.text);
    j.at("nextNodeId").get_to(resp.nextNodeId);
    if(j.contains("effects")) {
        j.at("effects").get_to(resp.effects);
    }
    if(j.contains("conditions")) {
        j.at("conditions").get_to(resp.conditions);
    }
}
