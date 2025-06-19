//
// Created by bison on 19-06-25.
//

#ifndef SANDBOX_RENDERING_H
#define SANDBOX_RENDERING_H

#include "raylib.h"

void DrawTexturedQuadWithVertexColors(Texture2D tex, Rectangle src, Rectangle dest, Color c1, Color c2, Color c3, Color c4);

void DrawTexturedQuadWithVertexColorsRotated(
        Texture2D tex,
        Rectangle src,
        Rectangle dest,
        Color c1, Color c2, Color c3, Color c4,
        Vector2 origin, float rotationDegrees);

#endif //SANDBOX_RENDERING_H
