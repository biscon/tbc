//
// Created by bison on 19-01-25.
//

#ifndef SANDBOX_MATHUTIL_H
#define SANDBOX_MATHUTIL_H

struct Vector2i {
    int x, y;
    Vector2i(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Vector2i& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vector2i& other) const { return !(*this == other); }
};


#endif //SANDBOX_MATHUTIL_H
