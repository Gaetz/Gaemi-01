#ifndef SPRITE_VERTEX_H
#define SPRITE_VERTEX_H

#include "maths.h"

struct SpriteVertex
{
    Vector2 position;
    Vector4 color;
    Vector2 texture;
    SpriteVertex(Vector2 position, Vector4 color, Vector2 texture = Vector2())
        : position(position), color(color), texture(texture) {}
};

#endif