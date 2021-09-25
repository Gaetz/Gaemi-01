//
// Created by gaetz on 25/09/2021.
//

#ifndef GAMETYPES_H
#define GAMETYPES_H

#include <functional>

struct GameType {
    //std::function<void()> onLoad;
    std::function<void()> onLoad;
    std::function<void(u32)> onUpdate;
    std::function<void()> onDraw;
    std::function<void()> onCleanup;
};

#endif //GAMETYPES_H
