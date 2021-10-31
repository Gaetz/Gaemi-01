//
// Created by gaetz on 30/10/2021.
//

#include "AssetManager.h"
#include "../render/vk/Texture.h"
#include "../render/RendererFrontEnd.h"
#include "../render/vk/Init.h"
#include "../Locator.h"

using engine::asset::AssetManager;

engine::asset::AssetManager::AssetManager(render::RendererFrontEnd& renderer) : renderer(renderer) {}

bool engine::asset::AssetManager::init() {
    Locator::provide(this);
    return true;
}

engine::render::vk::Texture& AssetManager::getTexture(const string& name) {
    return textures[name];
}

void AssetManager::loadTexture(const string& file, const string& name) {
    textures[name] = renderer.loadTexture(file);
}

void AssetManager::close() {
    renderer.waitIdle();
    textures.clear();
    meshes.clear();
}

engine::render::vk::Mesh& AssetManager::getMesh(const string& name) {
    return meshes[name];
}

void AssetManager::loadMesh(const string& file, const string& name) {
    render::vk::Mesh mesh;
    mesh.loadFromObj(file);
    meshes[name] = mesh;
    renderer.upload(meshes[name]);
}

engine::render::vk::Material& AssetManager::getMaterial(const string& name) {
    return materials[name];
}

void
AssetManager::createMaterial(VkPipeline pipelineP, VkPipelineLayout pipelineLayoutP, const string& name) {
    engine::render::vk::Material material {};
    material.pipeline = pipelineP;
    material.pipelineLayout = pipelineLayoutP;
    materials[name] = material;
}

