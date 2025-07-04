//
// Created by bison on 19-01-25.
//

#ifndef SANDBOX_MATHUTIL_H
#define SANDBOX_MATHUTIL_H

#include "raylib.h"
#include "json.hpp"

struct Vector2i {
    int x, y;
    Vector2i(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Vector2i& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vector2i& other) const { return !(*this == other); }
};

void to_json(nlohmann::json& j, const Vector2i& v);
void from_json(const nlohmann::json& j, Vector2i& v);

int DistanceSquared(Vector2i a, Vector2i b);
float Distance(Vector2i start, Vector2i end);
Vector2 CalculateDirection(Vector2 start, Vector2 end);
Vector2 CalculateDirection(Vector2i start, Vector2i end);
Vector2 ceilv(Vector2 v);
float GetRandomFloat01();

#endif //SANDBOX_MATHUTIL_H
