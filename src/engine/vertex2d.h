#include "color.h"

struct Position
{
    float x;
    float y;
};

struct UV
{
    float u;
    float v;
};

struct Vertex2D
{
    Position position;
    Color color;
    UV uv;

    void setPosition(float x, float y)
    {
        position.x = x;
        position.y = y;
    }

    void setColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
    {
        color.setColor(r, g, b, a);
    }

    void setUV(float u, float v)
    {
        uv.u = u;
        uv.v = v;
    }
};
