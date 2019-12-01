#include "maths.h"

struct SpriteVertex
{
    Vector2 position;
    Vector4 color;
    Vector2 texture;
    SpriteVertex(Vector2 position, Vector4 color, Vector2 texture = Vector2())
        : position(position), color(color), texture(texture) {}
};