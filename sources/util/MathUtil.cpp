//
// Created by bison on 19-01-25.
//

#include "MathUtil.h"
#include "raymath.h"
#include <cmath>
#include <random>

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

Vector2i ToDirectionVector(Vector2 dir) {
    if (fabsf(dir.x) > fabsf(dir.y)) {
        // Horizontal dominates
        return { (dir.x > 0) ? 1 : -1, 0 };
    } else {
        // Vertical dominates
        return { 0, (dir.y > 0) ? 1 : -1 };
    }
}

Color LerpColor(const Color& a, const Color& b, float t) {
    return {
            (unsigned char)(a.r + (b.r - a.r) * t),
            (unsigned char)(a.g + (b.g - a.g) * t),
            (unsigned char)(a.b + (b.b - a.b) * t),
            255
    };
}

float EaseInOutCubic(float t) {
    return t < 0.5f
           ? 4.0f * t * t * t
           : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}
