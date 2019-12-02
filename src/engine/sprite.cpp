#include "sprite.h"
#include "resource_manager.h"

Sprite::Sprite() : position(Vector2()), angle(0.f), textureName("tile")
{}

Sprite::~Sprite()
{}

void Sprite::draw(const Spritebatch& spritebatch)
{
    Vector4 rect = Vector4(position.x, position.y, 25.f, 25.f);
    Vector4 uvRect = Vector4(0.f, 0.f, 1.f, 1.f);
    spritebatch.draw(rect, uvRect, ResourceManager::getTexture(textureName).id, 0.f, Color::white);
    // draw(const Vector4& destRect, const Vector4& uvRect, GLuint texture, float depth, const Color& color);
}