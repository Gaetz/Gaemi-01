//
// Created by gaetz on 30/10/2021.
//

#include "Assets.h"
#include "../render/vk/Texture.h"
#include "../render/vk/Mesh.h"
#include "../render/vk/GameObject.h"
#include "../render/vk/Context.h"
#include "../render/vk/Shader.h"

using engine::asset::NullAssets;

void NullAssets::placeholderMessage() const {
    LOG(LogLevel::Warning) << "Usage of placeholder assets service.";
}

engine::render::vk::Texture& NullAssets::getTexture(const string& name) {
    placeholderMessage();
    auto* nullTexture = new engine::render::vk::Texture();
    return *nullTexture;
}

void NullAssets::loadTexture(const string& file, const string& name) {
    placeholderMessage();
}

engine::render::vk::Mesh& NullAssets::getMesh(const string& name) {
    placeholderMessage();
    auto* nullMesh = new engine::render::vk::Mesh();
    return *nullMesh;
}

void NullAssets::loadMesh(const string& file, const string& name) {
    placeholderMessage();
}

void NullAssets::createMaterial(const string& name, const string& shaderName, const string& textureName) {
    placeholderMessage();
}

engine::render::vk::Material& NullAssets::getMaterial(const string& name) {
    placeholderMessage();
    auto* nullMaterial = new engine::render::vk::Material();
    return *nullMaterial;
}

engine::render::vk::Shader& engine::asset::NullAssets::getShader(const string& name) {
    placeholderMessage();
    engine::render::vk::Shader* nullShader { nullptr };
    return *nullShader;
}

engine::render::vk::Material* engine::asset::NullAssets::setMaterial(engine::render::vk::Material& material, const string& name) {
    placeholderMessage();
    engine::render::vk::Material* nullMaterial { nullptr };
    return nullMaterial;
}

engine::render::vk::Shader*
engine::asset::NullAssets::setShader(engine::render::vk::Shader& shader, const string& name) {
    placeholderMessage();
    engine::render::vk::Shader* nullShader { nullptr };
    return nullShader;
}

bool engine::asset::NullAssets::shaderExists(const string& name) const {
    placeholderMessage();
    return  false;
}

bool engine::asset::NullAssets::materialExists(const string& name) const {
    placeholderMessage();
    return false;
}

