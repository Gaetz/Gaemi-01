//
// Created by gaetz on 30/10/2021.
//

#include "AssetManager.h"
#include "../render/vk/Texture.h"
#include "../render/RendererFrontEnd.h"
#include "../render/vk/Init.h"
#include "../render/vk/Shader.h"
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

engine::render::vk::Shader& AssetManager::getShader(const string& name) {
    return shaders[name];
}

void AssetManager::createMaterial(const string& name) {
    renderer.createMaterial(name);
}

void
engine::asset::AssetManager::setMaterial(engine::render::vk::Material& material, engine::render::vk::Shader& shader,
                                         const string& name) {
    shaders[name] = shader;
    material.shader = &shaders[name];
    materials[name] = material;
}

