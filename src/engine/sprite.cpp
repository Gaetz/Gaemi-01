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
    GLuint id =  ResourceManager::getTexture(textureName).id;
    spritebatch.draw(rect, uvRect, id, 0.0f, color);
    // draw(const Vector4& destRect, const Vector4& uvRect, GLuint texture, float depth, const Color& color);
}
