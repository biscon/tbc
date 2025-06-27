//
// Created by bison on 26-06-25.
//

#ifndef SANDBOX_UIDATA_H
#define SANDBOX_UIDATA_H

struct InventoryUiState {
    int scrollOffset = 0;
    int selectedIndex = -1;
    int hoveredIndex = -1;
    bool draggingScrollKnob = false;
    float dragOffsetY = 0;
};

struct UiState {
    InventoryUiState inventory;
    int selectedCharacter;
};

#endif //SANDBOX_UIDATA_H
