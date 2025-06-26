//
// Created by bison on 19-06-25.
//

#include "StringUtil.h"
#include <sstream>
#include <iomanip>

// Helper: Convert 6-char hex color string to raylib Color
Color HexToColor(const std::string& hex) {
    if (hex.length() != 6) return BLACK;

    unsigned int r, g, b;
    std::stringstream ss;

    ss << std::hex << hex.substr(0, 2);
    ss >> r;
    ss.clear(); ss.str("");

    ss << std::hex << hex.substr(2, 2);
    ss >> g;
    ss.clear(); ss.str("");

    ss << std::hex << hex.substr(4, 2);
    ss >> b;

    return { static_cast<unsigned char>(r),
             static_cast<unsigned char>(g),
             static_cast<unsigned char>(b),
             255 };
}

std::string TruncateWithEllipsis(const std::string& input, size_t maxLength) {
    if (input.length() > maxLength)
        return input.substr(0, maxLength) + "..";
    return input;
}