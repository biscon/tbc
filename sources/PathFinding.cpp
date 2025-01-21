//
// Created by bison on 21-01-25.
//

#include "PathFinding.h"
#include <queue>
#include <functional>
#include <algorithm>

struct Node {
    Vector2i position;
    int gCost, hCost;
    Node *parent;

    Node(Vector2i pos, int g, int h, Node *p = nullptr) : position(pos), gCost(g), hCost(h), parent(p) {}

    int fCost() const { return gCost + hCost; }
};

Vector2 PixelToGridPosition(float pixelX, float pixelY) {
    int gridX = static_cast<int>((pixelX) / 16.0f);
    int gridY = static_cast<int>((pixelY) / 16.0f);
    return {static_cast<float>(gridX), static_cast<float>(gridY)};
}

Vector2i PixelToGridPositionI(int pixelX, int pixelY) {
    int gridX = pixelX / 16.0f;
    int gridY = pixelY / 16.0f;
    return {gridX, gridY};
}


// Helper function: Convert grid position to pixel position
Vector2 GridToPixelPosition(int gridX, int gridY) {
    return {gridX * 16.0f + 8.0f, gridY * 16.0f + 8.0f};
}

bool IsTileOccupied(CombatState &combat, int x, int y, Character *exceptCharacter) {
    // Check if the tile is walkable, returning false if out of bounds
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return false;
    if(GetTileAt(combat.tileMap, NAV_LAYER, x, y) != 0) return false;
    // check if any characters are in the way
    for (auto &character: combat.playerCharacters) {
        // skip dead
        if (character->health <= 0) continue;
        Vector2 gridPos = PixelToGridPosition(character->sprite.player.position.x, character->sprite.player.position.y);
        if ((int) gridPos.x == x && (int) gridPos.y == y && character != exceptCharacter) {
            //TraceLog(LOG_WARNING, "Player character in the way, x: %d, y: %d", x, y);
            return false;
        }
    }
    for (auto &character: combat.enemyCharacters) {
        // skip dead
        if (character->health <= 0) continue;
        Vector2 gridPos = PixelToGridPosition(character->sprite.player.position.x, character->sprite.player.position.y);
        if ((int) gridPos.x == x && (int) gridPos.y == y && character != exceptCharacter) {
            //TraceLog(LOG_WARNING, "Enemy character in the way, x: %d, y: %d", x, y);
            return false;
        }
    }
    return true;
}

bool IsTileWalkable(CombatState &combat, int x, int y) {
    // Check if the tile is walkable, returning false if out of bounds
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return false;
    if(GetTileAt(combat.tileMap, NAV_LAYER, x, y) != 0) return false;
    return true;
}

bool InitPath(CombatState &combat, Path &path, Vector2i start, Vector2i end, Character *exceptCharacter) {
    if (!IsTileOccupied(combat, start.x, start.y, exceptCharacter) || !IsTileOccupied(combat, end.x, end.y,
                                                                                      exceptCharacter)) {
        TraceLog(LOG_WARNING, "Start or end position is blocked, startX: %d, startY: %d, endX: %d, endY: %d", start.x,
                 start.y, end.x, end.y);
        return false;  // If the start or end is blocked, return false
    }

    std::vector<std::vector<bool>> closedSet(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));
    std::vector<std::vector<Node *>> allNodes(GRID_WIDTH, std::vector<Node *>(GRID_HEIGHT, nullptr));
    std::priority_queue<Node *, std::vector<Node *>, std::function<bool(Node *, Node *)>> openSet(
            [](Node *a, Node *b) { return a->fCost() > b->fCost(); });

    Node *startNode = new Node(start, 0, std::abs(start.x - end.x) + std::abs(start.y - end.y));
    openSet.push(startNode);
    allNodes[start.x][start.y] = startNode;

    const std::vector<Vector2i> directions = {{0,  1},
                                              {1,  0},
                                              {0,  -1},
                                              {-1, 0}};

    while (!openSet.empty()) {
        Node *currentNode = openSet.top();
        openSet.pop();

        if (currentNode->position == end) {
            // Reconstruct the path from end to start
            path.path.clear();
            Node *temp = currentNode;
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

        for (const Vector2i &dir: directions) {
            int neighborX = currentNode->position.x + dir.x;
            int neighborY = currentNode->position.y + dir.y;

            if (neighborX < 0 || neighborX >= GRID_WIDTH || neighborY < 0 || neighborY >= GRID_HEIGHT) continue;
            if (!IsTileOccupied(combat, neighborX, neighborY, exceptCharacter)) continue;
            if (closedSet[neighborX][neighborY]) continue;

            int tentativeGCost = currentNode->gCost + 1;
            Node *neighborNode = allNodes[neighborX][neighborY];

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

bool InitPathIgnoreOccupied(CombatState &combat, Path &path, Vector2i start, Vector2i end) {
    if (!IsTileWalkable(combat, start.x, start.y) || !IsTileWalkable(combat, end.x, end.y)) {
        TraceLog(LOG_WARNING, "Start or end position is blocked, startX: %d, startY: %d, endX: %d, endY: %d", start.x,
                 start.y, end.x, end.y);
        return false;  // If the start or end is blocked, return false
    }

    std::vector<std::vector<bool>> closedSet(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));
    std::vector<std::vector<Node *>> allNodes(GRID_WIDTH, std::vector<Node *>(GRID_HEIGHT, nullptr));
    std::priority_queue<Node *, std::vector<Node *>, std::function<bool(Node *, Node *)>> openSet(
            [](Node *a, Node *b) { return a->fCost() > b->fCost(); });

    Node *startNode = new Node(start, 0, std::abs(start.x - end.x) + std::abs(start.y - end.y));
    openSet.push(startNode);
    allNodes[start.x][start.y] = startNode;

    const std::vector<Vector2i> directions = {{0,  1},
                                              {1,  0},
                                              {0,  -1},
                                              {-1, 0}};

    while (!openSet.empty()) {
        Node *currentNode = openSet.top();
        openSet.pop();

        if (currentNode->position == end) {
            // Reconstruct the path from end to start
            path.path.clear();
            Node *temp = currentNode;
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

        for (const Vector2i &dir: directions) {
            int neighborX = currentNode->position.x + dir.x;
            int neighborY = currentNode->position.y + dir.y;

            if (neighborX < 0 || neighborX >= GRID_WIDTH || neighborY < 0 || neighborY >= GRID_HEIGHT) continue;
            if (!IsTileWalkable(combat, neighborX, neighborY)) continue;
            if (closedSet[neighborX][neighborY]) continue;

            int tentativeGCost = currentNode->gCost + 1;
            Node *neighborNode = allNodes[neighborX][neighborY];

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

bool InitPathWithRange(CombatState &combat, Path &path, Vector2i start, Vector2i end, int range,
                       Character *exceptCharacter) {
    if (!IsTileOccupied(combat, start.x, start.y, exceptCharacter)) {
        TraceLog(LOG_WARNING, "Start position is blocked, startX: %d, startY: %d, endX: %d, endY: %d", start.x, start.y,
                 end.x, end.y);
        return false;  // If the start position is blocked, return false
    }

    std::vector<std::vector<bool>> closedSet(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));
    std::vector<std::vector<Node *>> allNodes(GRID_WIDTH, std::vector<Node *>(GRID_HEIGHT, nullptr));
    std::priority_queue<Node *, std::vector<Node *>, std::function<bool(Node *, Node *)>> openSet(
            [](Node *a, Node *b) { return a->fCost() > b->fCost(); });

    Node *startNode = new Node(start, 0, std::abs(start.x - end.x) + std::abs(start.y - end.y));
    openSet.push(startNode);
    allNodes[start.x][start.y] = startNode;

    const std::vector<Vector2i> directions = {{0,  1},
                                              {1,  0},
                                              {0,  -1},
                                              {-1, 0}};

    while (!openSet.empty()) {
        Node *currentNode = openSet.top();
        openSet.pop();

        // Check if the current node is within the specified range of the end position
        int distanceToEnd = std::abs(currentNode->position.x - end.x) + std::abs(currentNode->position.y - end.y);
        if (distanceToEnd <= range) {
            // Reconstruct the path up to the current node
            path.path.clear();
            Node *temp = currentNode;
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

        for (const Vector2i &dir: directions) {
            int neighborX = currentNode->position.x + dir.x;
            int neighborY = currentNode->position.y + dir.y;

            if (neighborX < 0 || neighborX >= GRID_WIDTH || neighborY < 0 || neighborY >= GRID_HEIGHT) continue;
            if (!IsTileOccupied(combat, neighborX, neighborY, exceptCharacter)) continue;
            if (closedSet[neighborX][neighborY]) continue;

            int tentativeGCost = currentNode->gCost + 1;
            Node *neighborNode = allNodes[neighborX][neighborY];

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
