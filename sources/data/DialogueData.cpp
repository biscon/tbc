//
// Created by bison on 10-06-25.
//

#include "DialogueData.h"

void to_json(nlohmann::json& j, const DialogueNode& node) {
    j = nlohmann::json {
            {"id", node.id},
            {"text", node.text},
            {"responseIds", node.responseIds}
    };
}

void from_json(const nlohmann::json& j, DialogueNode& node) {
    j.at("id").get_to(node.id);
    j.at("text").get_to(node.text);
    j.at("responseIds").get_to(node.responseIds);
}

void to_json(nlohmann::json& j, const DialogueResponse& resp) {
    j = nlohmann::json{
            {"id", resp.id},
            {"text", resp.text},
            {"nextNodeId", resp.nextNodeId},
            {"startQuestId", resp.startQuestId},
            {"completeQuestId", resp.completeQuestId}
    };
}

void from_json(const nlohmann::json& j, DialogueResponse& resp) {
    j.at("id").get_to(resp.id);
    j.at("text").get_to(resp.text);
    j.at("nextNodeId").get_to(resp.nextNodeId);
    j.at("startQuestId").get_to(resp.startQuestId);
    j.at("completeQuestId").get_to(resp.completeQuestId);
}
