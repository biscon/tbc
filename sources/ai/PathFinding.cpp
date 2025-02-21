//
// Created by bison on 21-01-25.
//

#include "PathFinding.h"
#include "raymath.h"
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

bool IsTileOccupied(Level &level, int x, int y, Character *exceptCharacter) {
    if(GetTileAt(level.tileMap, NAV_LAYER, x, y) != 0) return false;
    // check if any characters are in the way
    for (auto &character: level.playerCharacters) {
        // skip dead
        if (character->health <= 0) continue;
        Vector2 gridPos = PixelToGridPosition(GetCharacterSpritePosX(character->sprite), GetCharacterSpritePosY(character->sprite));
        if ((int) gridPos.x == x && (int) gridPos.y == y && character != exceptCharacter) {
            //TraceLog(LOG_WARNING, "Player character in the way, x: %d, y: %d", x, y);
            return false;
        }
    }
    for (auto &character: level.enemyCharacters) {
        // skip dead
        if (character->health <= 0) continue;
        Vector2 gridPos = PixelToGridPosition(GetCharacterSpritePosX(character->sprite), GetCharacterSpritePosY(character->sprite));
        if ((int) gridPos.x == x && (int) gridPos.y == y && character != exceptCharacter) {
            //TraceLog(LOG_WARNING, "Enemy character in the way, x: %d, y: %d", x, y);
            return false;
        }
    }
    return true;
}

bool IsTileWalkable(Level &level, int x, int y) {
    if(GetTileAt(level.tileMap, NAV_LAYER, x, y) != 0) return false;
    return true;
}

bool IsTileBlocking(Level &combat, int x, int y) {
    if(GetTileAt(combat.tileMap, NAV_LAYER, x, y) != 0) return true;
    return false;
}

bool InitPath(Level &level, Path &path, Vector2i start, Vector2i end, Character *exceptCharacter) {
    if (!IsTileOccupied(level, start.x, start.y, exceptCharacter) || !IsTileOccupied(level, end.x, end.y,
                                                                                     exceptCharacter)) {
        TraceLog(LOG_WARNING, "Start or end position is blocked, startX: %d, startY: %d, endX: %d, endY: %d", start.x,
                 start.y, end.x, end.y);
        return false;  // If the start or end is blocked, return false
    }

    std::vector<std::vector<bool>> closedSet(level.tileMap.width, std::vector<bool>(level.tileMap.height, false));
    std::vector<std::vector<Node *>> allNodes(level.tileMap.width, std::vector<Node *>(level.tileMap.height, nullptr));
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

            if (!IsTileOccupied(level, neighborX, neighborY, exceptCharacter)) continue;
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

bool InitPathIgnoreOccupied(Level &level, Path &path, Vector2i start, Vector2i end) {
    if (!IsTileWalkable(level, start.x, start.y) || !IsTileWalkable(level, end.x, end.y)) {
        TraceLog(LOG_WARNING, "Start or end position is blocked, startX: %d, startY: %d, endX: %d, endY: %d", start.x,
                 start.y, end.x, end.y);
        return false;  // If the start or end is blocked, return false
    }

    std::vector<std::vector<bool>> closedSet(level.tileMap.width, std::vector<bool>(level.tileMap.height, false));
    std::vector<std::vector<Node *>> allNodes(level.tileMap.width, std::vector<Node *>(level.tileMap.height, nullptr));
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

            if (!IsTileWalkable(level, neighborX, neighborY)) continue;
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

bool InitPathWithRange(Level &level, Path &path, Vector2i start, Vector2i end, int range,
                       Character *exceptCharacter) {
    if (!IsTileOccupied(level, start.x, start.y, exceptCharacter)) {
        TraceLog(LOG_WARNING, "Start position is blocked, startX: %d, startY: %d, endX: %d, endY: %d", start.x, start.y,
                 end.x, end.y);
        return false;  // If the start position is blocked, return false
    }

    std::vector<std::vector<bool>> closedSet(level.tileMap.width, std::vector<bool>(level.tileMap.height, false));
    std::vector<std::vector<Node *>> allNodes(level.tileMap.width, std::vector<Node *>(level.tileMap.height, nullptr));
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

            if (!IsTileOccupied(level, neighborX, neighborY, exceptCharacter)) continue;
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

bool InitPathWithRangePartial(Level &level, Path &path, Vector2i start, Vector2i end, int range,
                              Character *exceptCharacter) {
    if (!IsTileOccupied(level, start.x, start.y, exceptCharacter)) {
        TraceLog(LOG_WARNING, "Start position is blocked, startX: %d, startY: %d, endX: %d, endY: %d",
                 start.x, start.y, end.x, end.y);
        return false;  // If the start position is blocked, return false
    }

    std::vector<std::vector<bool>> closedSet(level.tileMap.width, std::vector<bool>(level.tileMap.height, false));
    std::vector<std::vector<Node *>> allNodes(level.tileMap.width, std::vector<Node *>(level.tileMap.height, nullptr));
    std::priority_queue<Node *, std::vector<Node *>, std::function<bool(Node *, Node *)>> openSet(
            [](Node *a, Node *b) { return a->fCost() > b->fCost(); });

    Node *startNode = new Node(start, 0, std::abs(start.x - end.x) + std::abs(start.y - end.y));
    openSet.push(startNode);
    allNodes[start.x][start.y] = startNode;

    const std::vector<Vector2i> directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    Node *farthestNode = startNode;  // Track the farthest reachable node
    int farthestDistance = std::abs(start.x - end.x) + std::abs(start.y - end.y);

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

        // Update the farthest node if this node is farther
        if (distanceToEnd < farthestDistance) {
            farthestNode = currentNode;
            farthestDistance = distanceToEnd;
        }

        closedSet[currentNode->position.x][currentNode->position.y] = true;

        for (const Vector2i &dir : directions) {
            int neighborX = currentNode->position.x + dir.x;
            int neighborY = currentNode->position.y + dir.y;

            if (!IsTileOccupied(level, neighborX, neighborY, exceptCharacter)) continue;
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

    // If no path to the destination was found, return the farthest reachable path
    path.path.clear();
    Node *temp = farthestNode;
    while (temp) {
        path.path.push_back(temp->position);
        temp = temp->parent;
    }
    std::reverse(path.path.begin(), path.path.end());
    path.cost = farthestNode->gCost;
    path.currentStep = 0;
    path.moveTime = 0.0f;
    path.moveSpeed = 0.15f;

    return false;  // Path to the destination was not found
}

bool IsCharacterAdjacentToPlayer(Character &player, Character &character) {
    Vector2i charPos = GetCharacterSpritePosI(character.sprite);
    Vector2i playerPos = GetCharacterSpritePosI(player.sprite);

    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    Vector2i playerGridPos = PixelToGridPositionI(playerPos.x, playerPos.y);
    return abs(playerGridPos.x - charGridPos.x) <= 1 && abs(playerGridPos.y - charGridPos.y) <= 1;
}

bool HasLineOfSight(Level &level, Vector2i start, Vector2i end) {
    int x0 = start.x;
    int y0 = start.y;
    int x1 = end.x;
    int y1 = end.y;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (true) {
        // Check if the current tile blocks line of sight
        if (IsTileBlocking(level, x0, y0)) {
            return false;
        }

        // If we reach the end position, return true
        if (x0 == x1 && y0 == y1) {
            return true;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

bool HasLineOfSight(Level &level, Vector2i start, Vector2i end, int maxDist) {
    int x0 = start.x;
    int y0 = start.y;
    int x1 = end.x;
    int y1 = end.y;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;
    int dist = 0;

    while (dist <= maxDist) {
        // Check if the current tile blocks line of sight
        if (IsTileBlocking(level, x0, y0)) {
            return false;
        }

        // If we reach the end position, return true
        if (x0 == x1 && y0 == y1) {
            return true;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }

        dist++;
    }

    return false; // Max distance reached without reaching the target
}

std::vector<Vector2i> FindFreePositionsCircular(Level& level, int x, int y, int radius) {
    std::vector<Vector2i> freePositions;

    // Iterate over the bounding box of the circle
    for (int offsetX = -radius; offsetX <= radius; ++offsetX) {
        for (int offsetY = -radius; offsetY <= radius; ++offsetY) {
            int checkX = x + offsetX;
            int checkY = y + offsetY;

            // Check if the position is within the playfield bounds
            if (checkX < 0 || checkX >= level.tileMap.width || checkY < 0 || checkY >= level.tileMap.height) continue;

            // Check if the position is within the circle's radius
            if (offsetX * offsetX + offsetY * offsetY <= radius * radius) {
                // Add the position if it is not occupied
                if (IsTileWalkable(level, checkX, checkY)) {
                    freePositions.push_back(Vector2i{checkX, checkY});
                }
            }
        }
    }

    // Sort the free positions by their distance from the center
    std::sort(freePositions.begin(), freePositions.end(), [x, y](const Vector2i &a, const Vector2i &b) {
        int distA = (a.x - x) * (a.x - x) + (a.y - y) * (a.y - y);
        int distB = (b.x - x) * (b.x - x) + (b.y - y) * (b.y - y);
        return distA > distB;
    });

    return freePositions;
}

std::vector<Character*> GetTargetsInLine(Level &level, Vector2i start, Vector2 direction, int range, Character* exceptCharacter) {
    std::vector<Character*> affectedCharacters;

    // Normalize the direction vector to ensure consistent movement
    direction = Vector2Normalize(direction);

    // Iterate through the range
    for (int step = 0; step < range; ++step) {
        // Calculate the tile position at the current step
        Vector2 currentPos = {
                start.x + direction.x * step,
                start.y + direction.y * step
        };
        Vector2i tilePos = { (int)roundf(currentPos.x), (int)roundf(currentPos.y) };

        // Check if the tile is within bounds
        if (tilePos.x < 0 || tilePos.x >= level.tileMap.width || tilePos.y < 0 || tilePos.y >= level.tileMap.height) {
            break; // Out of bounds
        }

        // Check if a character is on this tile
        for (auto &character : level.playerCharacters) {
            if(character == exceptCharacter) continue;
            // skip dead
            if (character->health <= 0) continue;
            Vector2i charPos = GetCharacterSpritePosI(character->sprite);
            Vector2i gridPos = PixelToGridPositionI(charPos.x, charPos.y);
            if (gridPos == tilePos) {
                affectedCharacters.push_back(character);
            }
        }

        for (auto &character : level.enemyCharacters) {
            if(character == exceptCharacter) continue;
            // skip dead
            if (character->health <= 0) continue;
            Vector2i charPos = GetCharacterSpritePosI(character->sprite);
            Vector2i gridPos = PixelToGridPositionI(charPos.x, charPos.y);
            if (gridPos == tilePos) {
                affectedCharacters.push_back(character);
            }
        }
    }
    return affectedCharacters;
}

