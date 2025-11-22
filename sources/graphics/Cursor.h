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

namespace Cursor {

    bool CreateFromPixels(CursorHandle& out,
                          const void* rgbaPixels,
                          int width,
                          int height,
                          int hotspotX,
                          int hotspotY);

    bool CreateFromImage(CursorHandle& out,
                         const Image& img,
                         int hotspotX,
                         int hotspotY);

    void Set(const CursorHandle& cursor);
    void ResetToSystemDefault();
    void Destroy(CursorHandle& cursor);

}

#endif //SANDBOX_CURSOR_H
