//
// Created by bison on 19-06-25.
//

#include <cmath>
#include "Rendering.h"
#include "rlgl.h"

void DrawTexturedQuadWithVertexColors(Texture2D tex, Rectangle src, Rectangle dest, Color c1, Color c2, Color c3, Color c4) {
    // Enable texture
    rlSetTexture(tex.id);
    rlBegin(RL_QUADS);

    // Top-left (v1)
    rlColor4ub(c1.r, c1.g, c1.b, c1.a);
    rlTexCoord2f(src.x / tex.width, src.y / tex.height);
    rlVertex2f(dest.x, dest.y);

    // Bottom-left (v4)
    rlColor4ub(c4.r, c4.g, c4.b, c4.a);
    rlTexCoord2f(src.x / tex.width, (src.y + src.height) / tex.height);
    rlVertex2f(dest.x, dest.y + dest.height);

    // Bottom-right (v3)
    rlColor4ub(c3.r, c3.g, c3.b, c3.a);
    rlTexCoord2f((src.x + src.width) / tex.width, (src.y + src.height) / tex.height);
    rlVertex2f(dest.x + dest.width, dest.y + dest.height);

    // Top-right (v2)
    rlColor4ub(c2.r, c2.g, c2.b, c2.a);
    rlTexCoord2f((src.x + src.width) / tex.width, src.y / tex.height);
    rlVertex2f(dest.x + dest.width, dest.y);

    rlEnd();
    rlSetTexture(0); // disable texture
}

void DrawTexturedQuadWithVertexColorsRotated(
        Texture2D tex,
        Rectangle src,
        Rectangle dest,
        Color c1, Color c2, Color c3, Color c4,
        Vector2 origin, float rotationDegrees) {

    float tx0 = src.x / tex.width;
    float ty0 = src.y / tex.height;
    float tx1 = (src.x + src.width) / tex.width;
    float ty1 = (src.y + src.height) / tex.height;

    float ox = dest.x + origin.x;
    float oy = dest.y + origin.y;

    float radians = rotationDegrees * (PI / 180.0f); // Convert to radians

    // Quad corners in counter-clockwise order (TL, BL, BR, TR)
    Vector2 quad[4] = {
            {0, 0},
            {0, dest.height},
            {dest.width, dest.height},
            {dest.width, 0}
    };

    for (int i = 0; i < 4; i++) {
        float dx = quad[i].x - origin.x;
        float dy = quad[i].y - origin.y;

        float rx = cosf(radians) * dx - sinf(radians) * dy;
        float ry = sinf(radians) * dx + cosf(radians) * dy;

        quad[i].x = ox + rx;
        quad[i].y = oy + ry;
    }

    rlSetTexture(tex.id);
    rlBegin(RL_QUADS);

    rlColor4ub(c1.r, c1.g, c1.b, c1.a); rlTexCoord2f(tx0, ty0); rlVertex2f(quad[0].x, quad[0].y); // TL
    rlColor4ub(c4.r, c4.g, c4.b, c4.a); rlTexCoord2f(tx0, ty1); rlVertex2f(quad[1].x, quad[1].y); // BL
    rlColor4ub(c3.r, c3.g, c3.b, c3.a); rlTexCoord2f(tx1, ty1); rlVertex2f(quad[2].x, quad[2].y); // BR
    rlColor4ub(c2.r, c2.g, c2.b, c2.a); rlTexCoord2f(tx1, ty0); rlVertex2f(quad[3].x, quad[3].y); // TR

    rlEnd();
    rlSetTexture(0);
}


void DrawTexturedQuadWithVertexColorsRotatedOLD(
        Texture2D tex,
        Rectangle src,
        Rectangle dest,
        Color c1, Color c2, Color c3, Color c4,
        Vector2 origin, float rotation) {

    float tx0 = src.x / tex.width;
    float ty0 = src.y / tex.height;
    float tx1 = (src.x + src.width) / tex.width;
    float ty1 = (src.y + src.height) / tex.height;

    float ox = dest.x + origin.x;
    float oy = dest.y + origin.y;



    // Define quad corners in counter-clockwise order
    Vector2 quad[4] = {
            {0, 0},                          // Top-left
            {0, dest.height},               // Bottom-left
            {dest.width, dest.height},      // Bottom-right
            {dest.width, 0}                 // Top-right
    };

    // Apply rotation around the origin
    for (int i = 0; i < 4; i++) {
        float dx = quad[i].x - origin.x;
        float dy = quad[i].y - origin.y;

        float rx = cosf(rotation) * dx - sinf(rotation) * dy;
        float ry = sinf(rotation) * dx + cosf(rotation) * dy;

        quad[i].x = ox + rx;
        quad[i].y = oy + ry;
    }

    rlSetTexture(tex.id);
    rlBegin(RL_QUADS);

    // Top-left (c1)
    rlColor4ub(c1.r, c1.g, c1.b, c1.a);
    rlTexCoord2f(tx0, ty0);
    rlVertex2f(quad[0].x, quad[0].y);

    // Bottom-left (c4)
    rlColor4ub(c4.r, c4.g, c4.b, c4.a);
    rlTexCoord2f(tx0, ty1);
    rlVertex2f(quad[1].x, quad[1].y);

    // Bottom-right (c3)
    rlColor4ub(c3.r, c3.g, c3.b, c3.a);
    rlTexCoord2f(tx1, ty1);
    rlVertex2f(quad[2].x, quad[2].y);

    // Top-right (c2)
    rlColor4ub(c2.r, c2.g, c2.b, c2.a);
    rlTexCoord2f(tx1, ty0);
    rlVertex2f(quad[3].x, quad[3].y);

    rlEnd();
    rlSetTexture(0);
}

