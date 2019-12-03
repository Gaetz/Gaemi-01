#include "sprite.h"
#include "resource_manager.h"

Sprite::Sprite() : position(Vector2(0.f, 0.f)), angle(0.f), textureName("tile")
{}

Sprite::~Sprite()
{}

void Sprite::draw(Spritebatch& spritebatch)
{
    Vector4 rect = Vector4(position.x, position.y, 0.1, 0.1f);
    Vector4 uvRect = Vector4(0.f, 0.f, 1.f, 1.f);
    Color color = Color::white;
    Texture2D tex =  ResourceManager::getTexture(textureName);
    spritebatch.draw(rect, uvRect, tex.id, 0.0f, color);
}
