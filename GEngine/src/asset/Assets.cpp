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

void NullAssets::placeholderMessage() {
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

void NullAssets::createMaterial(const string& name) {
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

void engine::asset::NullAssets::setMaterial(engine::render::vk::Material& material, engine::render::vk::Shader& shader,
                                            const string& name) {
    placeholderMessage();
}

