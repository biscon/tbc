//
// Created by bison on 16-01-25.
//

#include <algorithm>
#include <cmath>
#include <queue>
#include <functional>
#include "Grid.h"
#include "raylib.h"
#include "ui.h"
#include "raymath.h"

struct Node {
    Vector2i position;
    int gCost, hCost;
    Node* parent;
    Node(Vector2i pos, int g, int h, Node* p = nullptr) : position(pos), gCost(g), hCost(h), parent(p) {}
    int fCost() const { return gCost + hCost; }
};


static bool IsCharacterVisible(CombatState &combat, Character *character) {
    // Check if the character is visible (not blinking)
    for (auto & animation : combat.animations) {
        if (animation.type == AnimationType::Blink) {
            if (animation.state.blink.character == character) {
                if (!animation.state.blink.visible) {
                    return false;
                }
            }
        }
    }
    return true;
}

void InitGrid(GridState &gridState, SpriteAnimationManager &animationManager) {
    gridState.animationManager = &animationManager;
    gridState.moving = false;
    gridState.mode = GridMode::Normal;
    gridState.selectedCharacter = nullptr;
    gridState.selectedTile = { -1, -1 };
    gridState.path = {};
    gridState.floatingStatsCharacter = nullptr;
}

static void ResetGridState(GridState &gridState) {
    gridState.moving = false;
    gridState.mode = GridMode::Normal;
    gridState.selectedCharacter = nullptr;
    gridState.selectedTile = { -1, -1 };
    gridState.path = {};
}

Vector2 PixelToGridPosition(float pixelX, float pixelY) {
    int gridX = static_cast<int>((pixelX - GRID_CENTER_OFFSET) / 16.0f);
    int gridY = static_cast<int>((pixelY - GRID_CENTER_OFFSET) / 16.0f);
    return { static_cast<float>(gridX), static_cast<float>(gridY) };
}

Vector2 PixelToGridPositionNoOffset(float pixelX, float pixelY) {
    int gridX = static_cast<int>((pixelX) / 16.0f);
    int gridY = static_cast<int>((pixelY) / 16.0f);
    return { static_cast<float>(gridX), static_cast<float>(gridY) };
}

Vector2i PixelToGridPositionI(int pixelX, int pixelY) {
    int gridX = static_cast<int>(((float) pixelX - GRID_CENTER_OFFSET) / 16.0f);
    int gridY = static_cast<int>(((float) pixelY - GRID_CENTER_OFFSET) / 16.0f);
    return { gridX, gridY };
}

Vector2i PixelToGridPositionNoOffsetI(int pixelX, int pixelY) {
    int gridX = pixelX / 16.0f;
    int gridY = pixelY / 16.0f;
    return { gridX, gridY };
}


// Helper function: Convert grid position to pixel position
Vector2 GridToPixelPosition(int gridX, int gridY) {
    return { gridX * 16.0f + 8.0f, gridY * 16.0f + 8.0f };
}

// Helper function: Calculate vertical center offset in grid space
int CalculateVerticalCenterOffset(int gridHeight, int numCharacters) {
    int totalCharacterHeightInTiles = numCharacters;
    int emptySpaceInTiles = (gridHeight / 16) - totalCharacterHeightInTiles;
    return emptySpaceInTiles / 2; // Center the characters
}

// Main function: Set initial grid positions
void SetInitialGridPositions(GridState &gridState, CombatState &combat) {
    // Grid dimensions
    const int gridHeightPixels = 208;

    // Calculate vertical offsets in grid space
    int playerVerticalCenterOffset = CalculateVerticalCenterOffset(gridHeightPixels, (int) combat.playerCharacters.size());
    int enemyVerticalCenterOffset = CalculateVerticalCenterOffset(gridHeightPixels, (int) combat.enemyCharacters.size());

    // Set initial grid positions for player characters
    for (int i = 0; i < combat.playerCharacters.size(); i++) {
        int gridX = i % 2 + 1; // Stagger between gridx = 0 and gridx = 1
        int gridY = playerVerticalCenterOffset + i; // Row position
        combat.playerCharacters[i]->sprite.player.position = GridToPixelPosition(gridX, gridY);

        // Set initial animation to paused
        PlaySpriteAnimation(
                combat.playerCharacters[i]->sprite.player,
                GetCharacterAnimation(combat.playerCharacters[i]->sprite, SpriteAnimationType::WalkRight),
                true
        );
        combat.playerCharacters[i]->sprite.player.playing = false;
    }

    // Set initial grid positions for enemy characters
    for (int i = 0; i < combat.enemyCharacters.size(); i++) {
        int gridX = 28 - (i % 2); // Stagger between gridx = 29 and gridx = 28
        int gridY = enemyVerticalCenterOffset + i; // Row position
        combat.enemyCharacters[i]->sprite.player.position = GridToPixelPosition(gridX, gridY);

        // Set initial animation to paused
        PlaySpriteAnimation(
                combat.enemyCharacters[i]->sprite.player,
                GetCharacterAnimation(combat.enemyCharacters[i]->sprite, SpriteAnimationType::WalkLeft),
                true
        );
        combat.enemyCharacters[i]->sprite.player.playing = false;
    }
}

void DrawPathSelection(GridState &gridState, CombatState &combat) {
    // check if mouse is over tile
    Vector2 mousePos = GetMousePosition();
    Vector2 gridPos = PixelToGridPositionNoOffset(mousePos.x, mousePos.y);
    if(IsTileOccupied(combat, static_cast<int>(gridPos.x), static_cast<int>(gridPos.y), nullptr)) {
        gridState.selectedTile = gridPos;
        // calculate a path and draw it as lines
        GridPath path;
        Vector2i target = PixelToGridPositionNoOffsetI(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        if(InitPath(combat, path, PixelToGridPositionNoOffsetI((int) combat.currentCharacter->sprite.player.position.x, (int) combat.currentCharacter->sprite.player.position.y), target)) {
            Color pathColor = Fade(GREEN, gridState.highlightAlpha);
            if(path.cost > combat.currentCharacter->movePoints) {
                DrawStatusText(TextFormat("Not enough movement points (%d)", combat.currentCharacter->movePoints), WHITE, 10, 10);
                pathColor = Fade(RED, gridState.highlightAlpha);
                // Draw cross
                DrawLine(gridPos.x * 16, gridPos.y * 16+1, gridPos.x * 16 + 15, gridPos.y * 16 + 16, pathColor);
                DrawLine(gridPos.x * 16 + 15, gridPos.y * 16+1, gridPos.x * 16, gridPos.y * 16 + 16, pathColor);

            } else {
                DrawStatusText(TextFormat("Movement points %d/%d", path.cost, combat.currentCharacter->movePoints), YELLOW, 10, 10);
            }
            for(int i = 0; i < path.path.size()-1; i++) {
                Vector2 start = GridToPixelPosition(path.path[i].x, path.path[i].y);
                Vector2 end = GridToPixelPosition(path.path[i+1].x, path.path[i+1].y);
                DrawLineEx(start, end, 1, pathColor);
            }
            int gridX = static_cast<int>(gridPos.x);
            int gridY = static_cast<int>(gridPos.y);
            if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT) {
                DrawRectangleLinesEx(
                        Rectangle{
                                (gridPos.x * 16),
                                (gridPos.y * 16) + 1,
                                15, 15
                        },
                        1, pathColor
                );
            }
            // Check for a mouse click
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && path.cost <= combat.currentCharacter->movePoints) {
                gridState.mode = GridMode::Normal;
                gridState.path = path;
                gridState.moving = true;
                combat.currentCharacter->movePoints -= path.cost;
                // cap at zero
                if(combat.currentCharacter->movePoints < 0) {
                    combat.currentCharacter->movePoints = 0;
                }
                combat.turnState = TurnState::Move;

            }
            if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                ResetGridState(gridState);
                combat.turnState = TurnState::SelectAction;
            }
        }
    } else {
        DrawRectangleLinesEx(
                Rectangle{
                        (gridPos.x * 16),
                        (gridPos.y * 16) + 1,
                        15, 15
                },
                1, Fade(RED, gridState.highlightAlpha)
        );
        // Draw cross
        DrawLine(gridPos.x * 16, gridPos.y * 16+1, gridPos.x * 16 + 15, gridPos.y * 16 + 16, Fade(RED, gridState.highlightAlpha));
        DrawLine(gridPos.x * 16 + 15, gridPos.y * 16+1, gridPos.x * 16, gridPos.y * 16 + 16, Fade(RED, gridState.highlightAlpha));
    }
}

void DrawSelectCharacters(GridState& gridState, std::vector<Character*> &characters, Color color) {
    Vector2 mousePos = GetMousePosition();
    Vector2 gridPos = PixelToGridPositionNoOffset(mousePos.x, mousePos.y);
    for (auto &character: characters) {
        Vector2 gridPosCharacter = PixelToGridPositionNoOffset(character->sprite.player.position.x,
                                                               character->sprite.player.position.y);
        if ((int) gridPosCharacter.x == (int) gridPos.x && (int) gridPosCharacter.y == (int) gridPos.y) {
            DrawCircleLines(character->sprite.player.position.x, character->sprite.player.position.y, 10, Fade(color, gridState.highlightAlpha));
            // Draw plus
            DrawLine(gridPos.x * 16 + 8, gridPos.y * 16-1, gridPos.x * 16 + 8, gridPos.y * 16 + 17, Fade(color, gridState.highlightAlpha)); // Vertical line
            DrawLine(gridPos.x * 16 - 1, gridPos.y * 16 + 8, gridPos.x * 16 + 17, gridPos.y * 16 + 8, Fade(color, gridState.highlightAlpha)); // Horizontal line
            gridState.selectedCharacter = character;
        }
    }
}

void DrawSelectCharacter(GridState &gridState, CombatState &combat, bool onlyEnemies) {
    gridState.selectedCharacter = nullptr;
    if(!onlyEnemies) {
        DrawSelectCharacters(gridState, combat.playerCharacters, YELLOW);
    }
    DrawSelectCharacters(gridState, combat.enemyCharacters, RED);
    if(gridState.selectedCharacter != nullptr) {
        DrawStatusText(TextFormat("Selected: %s", gridState.selectedCharacter->name.c_str()), YELLOW, 10, 10);

        int charX = static_cast<int>(gridState.selectedCharacter->sprite.player.position.x);
        int charY = static_cast<int>(gridState.selectedCharacter->sprite.player.position.y);

        // calculate a path and draw it as lines
        GridPath path;
        Vector2i target = PixelToGridPositionNoOffsetI(charX, charY);
        if(InitPathIgnoreOccupied(combat, path, PixelToGridPositionNoOffsetI((int) combat.currentCharacter->sprite.player.position.x, (int) combat.currentCharacter->sprite.player.position.y), target)) {
            Color pathColor = Fade(RED, gridState.highlightAlpha);
            if(path.cost <= 2) {
                for(int i = 0; i < path.path.size()-1; i++) {
                    Vector2 start = GridToPixelPosition(path.path[i].x, path.path[i].y);
                    Vector2 end = GridToPixelPosition(path.path[i+1].x, path.path[i+1].y);
                    DrawLineEx(start, end, 1, pathColor);
                }
                // Check for a mouse click
                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    combat.turnState = TurnState::Waiting;
                    combat.waitTime = 0.25f;
                    combat.selectedCharacter = gridState.selectedCharacter;
                    if(combat.selectedSkill == nullptr)
                        combat.nextState = TurnState::Attack;
                    else
                        combat.nextState = TurnState::UseSkill;
                    ResetGridState(gridState);
                }
            } else {
                DrawStatusText(TextFormat("Too far away!"), WHITE, 25, 10);
            }
        }
    } else {
        DrawStatusText("Select a character", WHITE, 10, 10);
    }
}

void DrawTargetSelection(GridState &gridState, CombatState &combat, bool onlyEnemies) {
    DrawSelectCharacter(gridState, combat, onlyEnemies);
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        ResetGridState(gridState);
        combat.turnState = TurnState::SelectAction;
    }
}

// Function to draw the health bar
void DrawHealthBar(float x, float y, float width, float health, float maxHealth) {
    // Draw the health bar background (gray)
    DrawRectangle(x, y, width, 2, GRAY);
    // Draw the health bar foreground (green for positive health, red for damage)
    DrawRectangle(x, y, width * (health / maxHealth), 2, GREEN);
}


void DrawGridCharacters(GridState &state, CombatState &combat) {
    // Sort characters by y position
    std::vector<Character*> sortedCharacters;
    for (auto &character : combat.playerCharacters) {
        sortedCharacters.push_back(character);
    }
    for (auto &character : combat.enemyCharacters) {
        sortedCharacters.push_back(character);
    }
    std::sort(sortedCharacters.begin(), sortedCharacters.end(), [](Character* a, Character* b) {
        return a->sprite.player.position.y < b->sprite.player.position.y;
    });


    // Draw a highlight for the current character if not moving
    if (combat.currentCharacter != nullptr && !state.moving && state.mode == GridMode::Normal) {
        Color outlineColor = Fade(YELLOW, state.highlightAlpha);
        DrawRectangleLinesEx(
                Rectangle{
                        combat.currentCharacter->sprite.player.position.x - 9,
                        combat.currentCharacter->sprite.player.position.y - 18,
                        17, 24
                },
                1, outlineColor
        );
    }

    // Draw characters
    for (auto &character : sortedCharacters) {
        // Draw oval shadow underneath
        DrawEllipse(character->sprite.player.position.x, character->sprite.player.position.y, 6, 4, Fade(BLACK, 0.25f));

        if (IsCharacterVisible(combat, character)) {
            DrawSpriteAnimation(character->sprite.player);
        } else {
            character->sprite.player.tint = { 255, 255, 255, 64 }; // Semi-transparent tint
            DrawSpriteAnimation(character->sprite.player);
            character->sprite.player.tint = WHITE; // Reset tint
        }

        // Draw health bar
        DrawHealthBar(character->sprite.player.position.x - 8, character->sprite.player.position.y - 21, 15, character->health, character->maxHealth);
    }
}

void DrawGridLines(GridState &gridState, CombatState &combat) {
    // draw a line grid of 30x13 16x16 tiles taking up a rectangle of 480x208
    for(int i = 0; i < GRID_WIDTH+1; i++) {
        DrawLine(16*i, 0, 16*i, 208, Fade(BLACK, 0.15));
    }
    // draw vertical lines
    for(int i = 0; i < GRID_HEIGHT+1; i++) {
        DrawLine(0, 16*i, 480, 16*i, Fade(BLACK, 0.15));
    }
    if(gridState.mode == GridMode::SelectingTile) {
        DrawPathSelection(gridState, combat);
    }
    if(gridState.mode == GridMode::SelectingEnemyTarget) {
        DrawTargetSelection(gridState, combat, true);
    }
}

bool IsTileOccupied(CombatState &combat, int x, int y, Character *exceptCharacter) {
    // Check if the tile is walkable, returning false if out of bounds
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return false;
    // check if any characters are in the way
    for(auto &character : combat.playerCharacters) {
        Vector2 gridPos = PixelToGridPositionNoOffset(character->sprite.player.position.x, character->sprite.player.position.y);
        if((int) gridPos.x == x && (int) gridPos.y == y && character != exceptCharacter) {
            //TraceLog(LOG_WARNING, "Player character in the way, x: %d, y: %d", x, y);
            return false;
        }
    }
    for(auto &character : combat.enemyCharacters) {
        Vector2 gridPos = PixelToGridPositionNoOffset(character->sprite.player.position.x, character->sprite.player.position.y);
        if((int) gridPos.x == x && (int) gridPos.y == y && character != exceptCharacter) {
            //TraceLog(LOG_WARNING, "Enemy character in the way, x: %d, y: %d", x, y);
            return false;
        }
    }
    return true;
}

bool IsTileWalkable(CombatState &combat, int x, int y) {
    // Check if the tile is walkable, returning false if out of bounds
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return false;
    return true;
}

bool InitPath(CombatState &combat, GridPath &path, Vector2i start, Vector2i end) {
    if (!IsTileOccupied(combat, start.x, start.y, combat.currentCharacter) || !IsTileOccupied(combat, end.x, end.y,
                                                                                              combat.currentCharacter)) {
        TraceLog(LOG_WARNING, "Start or end position is blocked, startX: %d, startY: %d, endX: %d, endY: %d", start.x, start.y, end.x, end.y);
        return false;  // If the start or end is blocked, return false
    }

    std::vector<std::vector<bool>> closedSet(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));
    std::vector<std::vector<Node*>> allNodes(GRID_WIDTH, std::vector<Node*>(GRID_HEIGHT, nullptr));
    std::priority_queue<Node*, std::vector<Node*>, std::function<bool(Node*, Node*)>> openSet(
            [](Node* a, Node* b) { return a->fCost() > b->fCost(); });

    Node* startNode = new Node(start, 0, std::abs(start.x - end.x) + std::abs(start.y - end.y));
    openSet.push(startNode);
    allNodes[start.x][start.y] = startNode;

    const std::vector<Vector2i> directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

    while (!openSet.empty()) {
        Node* currentNode = openSet.top();
        openSet.pop();

        if (currentNode->position == end) {
            // Reconstruct the path from end to start
            path.path.clear();
            Node* temp = currentNode;
            while (temp) {
                path.path.push_back(temp->position);
                temp = temp->parent;
            }
            std::reverse(path.path.begin(), path.path.end());

            // Calculate cost and return true
            path.cost = currentNode->gCost;
            path.currentStep = 0;
            path.moveTime = 0.0f;
            path.moveSpeed = 0.15f;  // This can be adjusted based on game mechanics
            return true;
        }

        closedSet[currentNode->position.x][currentNode->position.y] = true;

        for (const Vector2i& dir : directions) {
            int neighborX = currentNode->position.x + dir.x;
            int neighborY = currentNode->position.y + dir.y;

            if (neighborX < 0 || neighborX >= GRID_WIDTH || neighborY < 0 || neighborY >= GRID_HEIGHT) continue;
            if (!IsTileOccupied(combat, neighborX, neighborY, combat.currentCharacter)) continue;
            if (closedSet[neighborX][neighborY]) continue;

            int tentativeGCost = currentNode->gCost + 1;
            Node* neighborNode = allNodes[neighborX][neighborY];

            if (!neighborNode || tentativeGCost < neighborNode->gCost) {
                neighborNode = new Node(Vector2i(neighborX, neighborY), tentativeGCost,
                                        std::abs(neighborX - end.x) + std::abs(neighborY - end.y), currentNode);

                allNodes[neighborX][neighborY] = neighborNode;
                openSet.push(neighborNode);
            }
        }
    }
    return false;  // Path not found
}

bool InitPathIgnoreOccupied(CombatState &combat, GridPath &path, Vector2i start, Vector2i end) {
    if (!IsTileWalkable(combat, start.x, start.y) || !IsTileWalkable(combat, end.x, end.y)) {
        TraceLog(LOG_WARNING, "Start or end position is blocked, startX: %d, startY: %d, endX: %d, endY: %d", start.x, start.y, end.x, end.y);
        return false;  // If the start or end is blocked, return false
    }

    std::vector<std::vector<bool>> closedSet(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));
    std::vector<std::vector<Node*>> allNodes(GRID_WIDTH, std::vector<Node*>(GRID_HEIGHT, nullptr));
    std::priority_queue<Node*, std::vector<Node*>, std::function<bool(Node*, Node*)>> openSet(
            [](Node* a, Node* b) { return a->fCost() > b->fCost(); });

    Node* startNode = new Node(start, 0, std::abs(start.x - end.x) + std::abs(start.y - end.y));
    openSet.push(startNode);
    allNodes[start.x][start.y] = startNode;

    const std::vector<Vector2i> directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

    while (!openSet.empty()) {
        Node* currentNode = openSet.top();
        openSet.pop();

        if (currentNode->position == end) {
            // Reconstruct the path from end to start
            path.path.clear();
            Node* temp = currentNode;
            while (temp) {
                path.path.push_back(temp->position);
                temp = temp->parent;
            }
            std::reverse(path.path.begin(), path.path.end());

            // Calculate cost and return true
            path.cost = currentNode->gCost;
            path.currentStep = 0;
            path.moveTime = 0.0f;
            path.moveSpeed = 0.15f;  // This can be adjusted based on game mechanics
            return true;
        }

        closedSet[currentNode->position.x][currentNode->position.y] = true;

        for (const Vector2i& dir : directions) {
            int neighborX = currentNode->position.x + dir.x;
            int neighborY = currentNode->position.y + dir.y;

            if (neighborX < 0 || neighborX >= GRID_WIDTH || neighborY < 0 || neighborY >= GRID_HEIGHT) continue;
            if (!IsTileWalkable(combat, neighborX, neighborY)) continue;
            if (closedSet[neighborX][neighborY]) continue;

            int tentativeGCost = currentNode->gCost + 1;
            Node* neighborNode = allNodes[neighborX][neighborY];

            if (!neighborNode || tentativeGCost < neighborNode->gCost) {
                neighborNode = new Node(Vector2i(neighborX, neighborY), tentativeGCost,
                                        std::abs(neighborX - end.x) + std::abs(neighborY - end.y), currentNode);

                allNodes[neighborX][neighborY] = neighborNode;
                openSet.push(neighborNode);
            }
        }
    }
    return false;  // Path not found
}

void UpdateGrid(GridState &gridState, CombatState &combat, float dt) {
    // Update the pulsing alpha
    if (gridState.increasing) {
        gridState.highlightAlpha = Lerp(gridState.highlightAlpha, 1.0f, dt * gridState.pulseSpeed);
        if (gridState.highlightAlpha >= 0.99f) {
            gridState.increasing = false;
        }
    } else {
        gridState.highlightAlpha = Lerp(gridState.highlightAlpha, 0.25f, dt * gridState.pulseSpeed);
        if (gridState.highlightAlpha <= 0.26f) {
            gridState.increasing = true;
        }
    }
    if (gridState.moving) {
        gridState.path.moveTime += dt;

        // Calculate the percentage of completion for the current step
        float t = gridState.path.moveTime / gridState.path.moveSpeed;

        if (gridState.path.currentStep < gridState.path.path.size() - 1) {
            // Get the current and next waypoint positions
            Vector2 start = GridToPixelPosition(
                    gridState.path.path[gridState.path.currentStep].x,
                    gridState.path.path[gridState.path.currentStep].y);
            Vector2 end = GridToPixelPosition(
                    gridState.path.path[gridState.path.currentStep + 1].x,
                    gridState.path.path[gridState.path.currentStep + 1].y);

            // Lerp the x and y components separately
            combat.currentCharacter->sprite.player.position.x = Lerp(start.x, end.x, t);
            combat.currentCharacter->sprite.player.position.y = Lerp(start.y, end.y, t);

            // Determine the direction of movement and set the appropriate animation
            if (fabs(end.x - start.x) > fabs(end.y - start.y)) {
                // Horizontal movement
                if (end.x > start.x) {
                    PlaySpriteAnimation(
                            combat.currentCharacter->sprite.player,
                            GetCharacterAnimation(combat.currentCharacter->sprite, SpriteAnimationType::WalkRight),
                            true);
                } else {
                    PlaySpriteAnimation(
                            combat.currentCharacter->sprite.player,
                            GetCharacterAnimation(combat.currentCharacter->sprite, SpriteAnimationType::WalkLeft),
                            true);
                }
            } else {
                // Vertical movement
                if (end.y > start.y) {
                    PlaySpriteAnimation(
                            combat.currentCharacter->sprite.player,
                            GetCharacterAnimation(combat.currentCharacter->sprite, SpriteAnimationType::WalkDown),
                            true);
                } else {
                    PlaySpriteAnimation(
                            combat.currentCharacter->sprite.player,
                            GetCharacterAnimation(combat.currentCharacter->sprite, SpriteAnimationType::WalkUp),
                            true);
                }
            }

            // Check if we have completed the current step
            if (gridState.path.moveTime >= gridState.path.moveSpeed) {
                gridState.path.moveTime = 0.0f;
                gridState.path.currentStep++;

                // If the last step is reached, stop moving
                if (gridState.path.currentStep >= gridState.path.path.size() - 1) {
                    gridState.moving = false;
                    combat.currentCharacter->sprite.player.playing = false;
                    SetFrame(combat.currentCharacter->sprite.player, 0);
                    // set final position
                    auto finalPos = gridState.path.path[gridState.path.path.size()-1];
                    combat.currentCharacter->sprite.player.position = GridToPixelPosition(finalPos.x, finalPos.y);
                    combat.turnState = TurnState::SelectAction;
                }
            }
        }
    }

    // Update animations for all characters
    for (auto &character : combat.playerCharacters) {
        UpdateSpriteAnimation(character->sprite.player, dt);
    }
    for (auto &character : combat.enemyCharacters) {
        UpdateSpriteAnimation(character->sprite.player, dt);
    }
}

void DrawGrid(GridState &gridState, CombatState &combat) {
    DrawGridLines(gridState, combat);
    DrawGridCharacters(gridState, combat);
    // get mouse position
    gridState.floatingStatsCharacter = nullptr;
    Vector2 mousePos = GetMousePosition();
    Vector2 gridPos = PixelToGridPositionNoOffset(mousePos.x, mousePos.y);
    // check if mouse is over character
    for(auto &character : combat.turnOrder) {
        Vector2 gridPosCharacter = PixelToGridPositionNoOffset(character->sprite.player.position.x, character->sprite.player.position.y);
        if((int) gridPosCharacter.x == (int) gridPos.x && (int) gridPosCharacter.y == (int) gridPos.y) {
            gridState.floatingStatsCharacter = character;
        }
    }
}


