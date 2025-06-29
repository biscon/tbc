//
// Created by bison on 26-06-25.
//

#include "UiData.h"

ClickRegion CreateClickRegion(Rectangle rect) {
    ClickRegion region;
    region.rect = rect;
    return region;
}