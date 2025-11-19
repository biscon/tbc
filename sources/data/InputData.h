//
// Created by bison on 18-11-25.
//

#ifndef SANDBOX_INPUTDATA_H
#define SANDBOX_INPUTDATA_H

#include "raylib.h"
#include <vector>

enum class InputEventType {
    MouseClick,
    KeyPressed,
    KeyReleased,
    Any
};

struct MouseClickEvent {
    Vector2 screenPos;
    Vector2 worldPos;
    int button;       // MOUSE_LEFT_BUTTON, etc
    bool doubleClick;
};

struct KeyEvent {
    int key;          // KEY_A, KEY_SPACE, etc
};

struct InputEvent {
    InputEventType type;
    bool handled = false;  // ← consumer sets this

    union {
        MouseClickEvent mouse;
        KeyEvent key;
    };
};

struct InputData {
    std::vector<InputEvent> events;

    // double-click logic
    float lastClickTime = -1.0f;
    float doubleClickThreshold = 0.3f;
};

#endif //SANDBOX_INPUTDATA_H
