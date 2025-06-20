//
// Created by bison on 19-01-25.
//

#include "MathUtil.h"
#include "raymath.h"
#include <cmath>

void to_json(nlohmann::json& j, const Vector2i& v) {
    j = nlohmann::json::array({ v.x, v.y });
}

void from_json(const nlohmann::json& j, Vector2i& v) {
    if (!j.is_array() || j.size() != 2) {
        throw std::invalid_argument("Vector2i JSON must be an array of 2 integers");
    }
    v.x = j.at(0).get<int>();
    v.y = j.at(1).get<int>();
}

int DistanceSquared(Vector2i a, Vector2i b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    return dx * dx + dy * dy;
}

float Distance(Vector2i start, Vector2i end) {
    int dx = end.x - start.x;
    int dy = end.y - start.y;
    return sqrtf(dx * dx + dy * dy);
}

Vector2 CalculateDirection(Vector2 start, Vector2 end) {
    return Vector2Normalize(Vector2Subtract(end, start));
}

Vector2 CalculateDirection(Vector2i start, Vector2i end) {
    Vector2 s = { static_cast<float>(start.x), static_cast<float>(start.y) };
    Vector2 e = { static_cast<float>(end.x), static_cast<float>(end.y) };
    return Vector2Normalize(Vector2Subtract(e, s));
}

Vector2 ceilv(Vector2 v) {
    return { ceilf(v.x), ceilf(v.y) };
}