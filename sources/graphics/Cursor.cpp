//
// Created by bison on 21/11/2025.
//

#include "Cursor.h"

#include "Cursor.h"
#include <cstring> // memcpy

// ------------------------------
// RAYLIB HOOK (Linux)
// ------------------------------
#if defined(__linux__)
#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

// You must fill these from your platform layer.
// With raylib, do this after InitWindow():
//    Display* d = (Display*)GetWindowHandle();
//    Window w = DefaultRootWindow(d);
Display* g_X11Display = nullptr;
Window   g_X11Window  = 0;

#endif

#if defined(_WIN32)
//#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


// ------------------------------
// RAYLIB IMAGE (if available)
// ------------------------------
extern "C" {
struct Image {
    void* data;
    int width;
    int height;
    int mipmaps;
    int format;
};
}

namespace Cursor {

// ======================================================
// HELPERS
// ======================================================

// Convert RGBA → premultiplied BGRA
    static void ConvertToBGRA_Premul(const uint8_t* src, uint8_t* dst, int w, int h)
    {
        const size_t count = w * h;
        for (size_t i = 0; i < count; i++) {
            uint8_t r = src[i*4 + 0];
            uint8_t g = src[i*4 + 1];
            uint8_t b = src[i*4 + 2];
            uint8_t a = src[i*4 + 3];

            float af = a / 255.0f;

            dst[i*4 + 0] = uint8_t(b * af);
            dst[i*4 + 1] = uint8_t(g * af);
            dst[i*4 + 2] = uint8_t(r * af);
            dst[i*4 + 3] = a; // alpha kept as-is
        }
    }


// ======================================================
// WINDOWS IMPLEMENTATION
// ======================================================
#if defined(_WIN32)
    bool CreateFromPixels(CursorHandle& out,
                          const void* rgbaPixels,
                          int width,
                          int height,
                          int hotspotX,
                          int hotspotY)
    {
        if (!rgbaPixels || width <= 0 || height <= 0)
            return false;

        // Convert to premul BGRA
        uint8_t* bgra = new uint8_t[width * height * 4];
        ConvertToBGRA_Premul((const uint8_t*)rgbaPixels, bgra, width, height);

        // Create bitmap
        HBITMAP hColor = CreateBitmap(width, height, 1, 32, bgra);

        delete[] bgra;

        if (!hColor)
            return false;

        ICONINFO ii = {};
        ii.fIcon    = FALSE;
        ii.xHotspot = hotspotX;
        ii.yHotspot = hotspotY;
        ii.hbmMask  = nullptr;
        ii.hbmColor = hColor;

        HCURSOR hc = CreateIconIndirect(&ii);

        DeleteObject(hColor);

        if (!hc)
            return false;

        out.native = hc;
        return true;
    }

    void Set(const CursorHandle& cursor)
    {
        if (cursor.native)
            ::SetCursor((HCURSOR)cursor.native);
    }

    void ResetToSystemDefault()
    {
        ::SetCursor(LoadCursor(nullptr, IDC_ARROW));
    }

    void Destroy(CursorHandle& cursor)
    {
        if (cursor.native) {
            DestroyIcon((HCURSOR)cursor.native);
            cursor.native = nullptr;
        }
    }

#endif // windows



// ======================================================
// LINUX (X11 + Xcursor) IMPLEMENTATION
// ======================================================
#if defined(__linux__)

    bool CreateFromPixels(CursorHandle& out,
                      const void* rgbaPixels,
                      int width,
                      int height,
                      int hotspotX,
                      int hotspotY)
{
    if (!g_X11Display || !g_X11Window)
        return false;

    XcursorImage* img = XcursorImageCreate(width, height);
    if (!img) return false;

    img->xhot = hotspotX;
    img->yhot = hotspotY;

    memcpy(img->pixels, rgbaPixels, width * height * 4);

    Cursor cur = XcursorImageLoadCursor(g_X11Display, img);
    XcursorImageDestroy(img);

    if (!cur)
        return false;

    out.native = (void*)(uintptr_t)cur;
    return true;
}

void Set(const CursorHandle& cursor)
{
    if (cursor.native) {
        XDefineCursor(g_X11Display, g_X11Window, (Cursor)(uintptr_t)cursor.native);
        XFlush(g_X11Display);
    }
}

void ResetToSystemDefault()
{
    if (!g_X11Display || !g_X11Window) return;
    XUndefineCursor(g_X11Display, g_X11Window);
    XFlush(g_X11Display);
}

void Destroy(CursorHandle& cursor)
{
    if (cursor.native) {
        XFreeCursor(g_X11Display, (Cursor)(uintptr_t)cursor.native);
        cursor.native = nullptr;
    }
}

#endif // linux



// ======================================================
// MACOS IMPLEMENTATION
// ======================================================
#if defined(__APPLE__)
    #import <Cocoa/Cocoa.h>

bool CreateFromPixels(CursorHandle& out,
                      const void* rgbaPixels,
                      int width,
                      int height,
                      int hotspotX,
                      int hotspotY)
{
    uint8_t* plane = (uint8_t*)rgbaPixels;

    NSBitmapImageRep* rep =
        [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:&plane
            pixelsWide:width
            pixelsHigh:height
            bitsPerSample:8
            samplesPerPixel:4
            hasAlpha:YES
            isPlanar:NO
            colorSpaceName:NSDeviceRGBColorSpace
            bytesPerRow:width * 4
            bitsPerPixel:32];

    if (!rep)
        return false;

    NSImage* img = [[NSImage alloc] initWithSize:NSMakeSize(width, height)];
    [img addRepresentation:rep];

    NSCursor* cursor =
        [[NSCursor alloc] initWithImage:img
                                hotSpot:NSMakePoint(hotspotX, hotspotY)];

    if (!cursor)
        return false;

    out.native = (void*)CFBridgingRetain(cursor);
    return true;
}

void Set(const CursorHandle& cursor)
{
    if (cursor.native) {
        NSCursor* c = (__bridge NSCursor*)cursor.native;
        [c set];
    }
}

void ResetToSystemDefault()
{
    [[NSCursor arrowCursor] set];
}

void Destroy(CursorHandle& cursor)
{
    if (cursor.native) {
        CFBridgingRelease(cursor.native);
        cursor.native = nullptr;
    }
}

#endif // apple



// ======================================================
// GENERIC CreateFromImage (uses raylib Image)
// ======================================================
    bool CreateFromImage(CursorHandle& out,
                         const Image& img,
                         int hotspotX,
                         int hotspotY)
    {
        if (!img.data)
            return false;

        return CreateFromPixels(out,
                                img.data,
                                img.width,
                                img.height,
                                hotspotX,
                                hotspotY);
    }

} // namespace Cursor
