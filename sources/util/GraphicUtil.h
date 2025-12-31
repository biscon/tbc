//
// Created by bison on 20-08-25.
//

#ifndef SANDBOX_GRAPHICUTIL_H
#define SANDBOX_GRAPHICUTIL_H

#include <string>
#include "raylib.h"

Texture2D LoadTextureLinearized(const char* fileName, bool premultiplyAlpha);
Texture2D LoadTexturePreMultiplied(const char* fileName);
Vector3 ColorToVec3(const std::string& hex);

#endif //SANDBOX_GRAPHICUTIL_H
