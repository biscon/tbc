//
// Created by bison on 21/11/2025.
//

#ifndef SANDBOX_CURSOR_H
#define SANDBOX_CURSOR_H

#include <cstdint>

struct Image; // forward for raylib's Image

struct CursorHandle {
    void* native = nullptr;
};

void InitCursorPlatform();

bool CursorCreateFromPixels(CursorHandle& out,
                      const void* rgbaPixels,
                      int width,
                      int height,
                      int hotspotX,
                      int hotspotY);

bool CursorCreateFromImage(CursorHandle& out,
                     const Image& img,
                     int hotspotX,
                     int hotspotY);

void CursorSet(const CursorHandle& cursor);
void CursorResetToSystemDefault();
void CursorDestroy(CursorHandle& cursor);



#endif //SANDBOX_CURSOR_H
