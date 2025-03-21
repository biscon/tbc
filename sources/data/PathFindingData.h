//
// Created by bison on 14-03-25.
//

#ifndef SANDBOX_PATHFINDINGDATA_H
#define SANDBOX_PATHFINDINGDATA_H

#include <vector>
#include "util/MathUtil.h"

struct Path {
    std::vector<Vector2i> path;
    int cost; // cost of the path in distance travelled
    int currentStep;
    float moveTime;
    float moveSpeed;
};

#endif //SANDBOX_PATHFINDINGDATA_H
