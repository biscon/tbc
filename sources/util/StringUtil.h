//
// Created by bison on 19-06-25.
//

#ifndef SANDBOX_STRINGUTIL_H
#define SANDBOX_STRINGUTIL_H

#include <string>
#include "raylib.h"

Color HexToColor(const std::string& hex);
std::string TruncateWithEllipsis(const std::string& input, size_t maxLength);

#endif //SANDBOX_STRINGUTIL_H
