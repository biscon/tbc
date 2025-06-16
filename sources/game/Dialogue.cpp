//
// Created by bison on 11-06-25.
//

#include <fstream>
#include <sstream>
#include "Dialogue.h"
#include "raylib.h"
#include "util/GameEventQueue.h"

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

std::string WrapText(const std::string& text, float maxWidth, int fontSize) {
    std::istringstream iss(text);
    std::string word, line, result;

    while (iss >> word) {
        std::string test = line.empty() ? word : line + " " + word;
        int width = MeasureText(test.c_str(), fontSize);
        if (width > maxWidth) {
            result += line + "\n";
            line = word;
        } else {
            line = test;
        }
    }
    result += line;
    return result;
}

Vector2 MeasureMultilineText(const char* text, int fontSize, float maxWidth) {
    Vector2 size = {0};
    int lineCount = 0;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        int w = MeasureText(line.c_str(), fontSize);
        if (w > size.x) size.x = (float)w;
        size.y += fontSize;
        ++lineCount;
    }
    return size;
}

void DrawTextRec(const std::string& text, float x, float y, int fontSize, Color color) {
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        DrawText(line.c_str(), (int)x, (int)y, fontSize, color);
        y += (float) fontSize;
    }
}

void DrawMultilineText(const char* text, float x, float y, int fontSize, Color color) {
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        DrawText(line.c_str(), (int)x, (int)y, fontSize, color);
        y += (float) fontSize;
    }
}

void RenderDialogueUI(GameData& data) {
    auto& dlg = data.dialogueData;
    if (dlg.dialogueFade <= 0.0f) return;

    //const int screenWidth = 480;
    //const int screenHeight = 270;

    const Color overlay = Color{0, 0, 0, (unsigned char)(dlg.dialogueFade * 255 * 0.4f)};
    DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, overlay);

    if (dlg.currentDialogueNode == -1) return;

    const DialogueNode& node = dlg.dialogueNodes[dlg.currentDialogueNode];

    // Animation frame
    FrameInfo fi = GetFrameInfo(data.spriteData, dlg.idleAnimPlayer);
    float imgWidth = fi.srcRect.width;
    float imgHeight = fi.srcRect.height;
    float boxPad = 4.0f;
    float halfBoxPad = boxPad / 2.0f;
    Rectangle imgRect = {(gameScreenWidthF/2.0f - imgWidth/2.0f)-halfBoxPad, 50 - halfBoxPad, imgWidth+boxPad, imgHeight+boxPad};
    //DrawRectangleRec(imgRect, BLACK);
    DrawRectangleRounded(imgRect, 0.1f, 16, BLACK);
    DrawSpriteAnimation(data.spriteData, dlg.idleAnimPlayer, gameScreenWidthF/2.0f - imgWidth/2.0f, 50 + halfBoxPad);
    //DrawRectangleLinesEx(imgRect, halfBoxPad, DARKGRAY);
    DrawRectangleRoundedLinesEx(imgRect, 0.1f, 16, halfBoxPad, DARKGRAY);

    // Draw NPC name centered above
    std::string npcName = data.charData.name[dlg.currentNpc];
    int nameFontSize = 20;
    int nameWidth = MeasureText(npcName.c_str(), nameFontSize);
    DrawText(npcName.c_str(), gameScreenWidth / 2 - nameWidth / 2, 16, nameFontSize, WHITE);

    // Wrap and center dialogue text
    float dlgTextWidth = gameScreenWidth - 40;
    std::string wrappedText = WrapText(node.text, dlgTextWidth, 10);
    Vector2 textSize = MeasureMultilineText(wrappedText.c_str(), 10, dlgTextWidth);
    DrawTextRec(wrappedText, gameScreenWidth / 2 - textSize.x / 2, imgRect.y + imgRect.height + 20, 10, YELLOW);

    // Render responses from bottom-up
    dlg.responseRects.clear();
    float maxWidth = gameScreenWidth - 40;
    int fontSize = 10;
    float spacing = 2.0f;

    const float padX = 4.0f;
    const float padY = 2.0f;

    float totalHeight = 0;
    std::vector<Vector2> responseSizes;
    std::vector<std::string> responseText;

    for (int responseId : node.responseIds) {
        const std::string& text = dlg.dialogueResponses[responseId].text;
        std::string wrapped = WrapText(text, maxWidth - padX * 2, fontSize);
        Vector2 size = MeasureMultilineText(wrapped.c_str(), fontSize, maxWidth - padX * 2);
        responseText.push_back(wrapped);
        responseSizes.push_back(size);
        totalHeight += size.y + padY * 2 + spacing;
    }

    float startY = gameScreenHeight - totalHeight - 16;
    Vector2 mouse = GetMousePosition();

    for (size_t i = 0; i < node.responseIds.size(); ++i) {
        float y = startY;
        float x = 20;
        float w = maxWidth;
        float h = responseSizes[i].y + padY * 2;

        Rectangle rect = {x, y, w, h};
        dlg.responseRects.push_back(rect);

        bool hovered = CheckCollisionPointRec(mouse, rect);
        DrawRectangleRec(rect, Color{0, 0, 0, 100});
        if (hovered) DrawRectangleLinesEx(rect, 1.0f, WHITE);

        DrawMultilineText(responseText[i].c_str(), x + padX, y + padY, fontSize, hovered ? YELLOW : WHITE);
        startY += h + spacing;
    }
}


void UpdateDialogue(GameData& data, float dt) {
    auto& dlg = data.dialogueData;

    const float FADE_SPEED = 2.0f; // 0.5s to fade in/out

    if (dlg.currentDialogueNode != -1 && dlg.dialogueFade < 1.0f) {
        dlg.dialogueFade += dt * FADE_SPEED;
        if (dlg.dialogueFade > 1.0f) dlg.dialogueFade = 1.0f;
    } else if (dlg.currentDialogueNode == -1 && dlg.dialogueFade > 0.0f) {
        dlg.dialogueFade -= dt * FADE_SPEED;
        if (dlg.dialogueFade < 0.0f) dlg.dialogueFade = 0.0f;
    }
    if(dlg.idleAnimPlayer != -1 && data.state == GameState::DIALOGUE) {
        UpdateSpriteAnimation(data.spriteData, dlg.idleAnimPlayer, dt);
    }
}

static void handlePlayerResponse(GameData& data, int responseId, GameEventQueue& eventQueue) {
    auto& dlg = data.dialogueData;
    auto& response = dlg.dialogueResponses[responseId];

    // Start quest if needed
    if (!response.startQuestId.empty()) {
        auto& quest = data.quests[response.startQuestId];
        if (quest.status == QuestStatus::NotStarted) {
            quest.status = QuestStatus::Active;
            quest.stage = 0;
            // You could log or notify the player here
            printf("Quest started: %s\n", response.startQuestId.c_str());
        }
    }

    // Complete quest if needed
    if (!response.completeQuestId.empty()) {
        auto& quest = data.quests[response.completeQuestId];
        if (quest.status == QuestStatus::Active) {
            quest.status = QuestStatus::Completed;
            // Notify player or reward here
            printf("Quest completed: %s\n", response.completeQuestId.c_str());
        }
    }

    // Advance dialogue or end
    dlg.currentDialogueNode = response.nextNodeId;
    if (dlg.currentDialogueNode == -1) {
        PublishEndDialogueEvent(eventQueue, dlg.currentNpc);
        dlg.currentNpc = -1; // Exit dialogue mode
        //dlg.dialogueFade = 0.0f;
        //dlg.fadingOut = true;
    }
}


void HandleDialogueInput(GameData& data, GameEventQueue& eventQueue) {
    auto& dlg = data.dialogueData;
    if (dlg.currentDialogueNode == -1) return;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();

        for (size_t i = 0; i < dlg.responseRects.size(); ++i) {
            if (CheckCollisionPointRec(mouse, dlg.responseRects[i])) {
                const DialogueNode& node = dlg.dialogueNodes.at(dlg.currentDialogueNode);
                int responseId = node.responseIds[i];
                handlePlayerResponse(data, responseId, eventQueue);
                break;
            }
        }
    }
}
