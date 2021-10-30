//
// Created by gaetz on 30/10/2021.
//

#include "Assets.h"
#include "../render/vk/Texture.h"
#include "../render/vk/Mesh.h"
#include "../render/vk/GameObject.h"
#include "../render/vk/Context.h"

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

void
NullAssets::createMaterial(VkPipeline pipelineP, VkPipelineLayout pipelineLayoutP, const string& name) {
    placeholderMessage();
}

engine::render::vk::Material& NullAssets::getMaterial(const string& name) {
    placeholderMessage();
    auto* nullMaterial = new engine::render::vk::Material();
    return *nullMaterial;
}
