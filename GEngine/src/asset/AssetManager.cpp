//
// Created by gaetz on 30/10/2021.
//

#include "AssetManager.h"
#include "../render/vk/Texture.h"
#include "../render/RendererFrontEnd.h"
#include "../render/vk/Init.h"
#include "../render/vk/Shader.h"
#include "../Locator.h"
#include "../../../externals/etc2comp/EtcLib/Etc/EtcImage.h"
#include "../../../externals/etc2comp/EtcLib/EtcCodec/EtcErrorMetric.h"
#include "../../../externals/etc2comp/EtcTool/EtcFile.h"
#include "../../../externals/etc2comp/EtcLib/Etc/Etc.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>
#include <stb_image.h>
#include <thread>

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

    // Assimp test
    //const aiScene* scene = aiImportFile( "data/rubber_duck/scene.gltf", aiProcess_Triangulate );

    // ETC test
    /*
    int w, h, comp;
    const uint8_t* img = stbi_load( "../../assets/default.png", &w, &h, &comp, 4 );

    std::vector<float> rgbaf;

    for ( int i = 0; i != w * h * 4; i+=4 )
    {
        rgbaf.push_back( img[i+0] / 255.0f );
        rgbaf.push_back( img[i+1] / 255.0f );
        rgbaf.push_back( img[i+2] / 255.0f );
        rgbaf.push_back( img[i+3] / 255.0f );
    }

    const auto etcFormat = Etc::Image::Format::RGB8;
    const auto errorMetric = Etc::ErrorMetric::BT709;

    Etc::Image image( rgbaf.data(), w, h, errorMetric );

    image.Encode( etcFormat, errorMetric, ETCCOMP_DEFAULT_EFFORT_LEVEL, std::thread::hardware_concurrency(), 1024 );

    Etc::File etcFile(
            "image.ktx",
            Etc::File::Format::KTX,
            etcFormat,
            image.GetEncodingBits(),
            image.GetEncodingBitsBytes(),
            image.GetSourceWidth(),
            image.GetSourceHeight(),
            image.GetExtendedWidth(),
            image.GetExtendedHeight()
    );
    etcFile.Write();
     */
}

void AssetManager::close() {
    renderer.waitIdle();
    textures.clear();
    meshes.clear();
    materials.clear();
    for(auto& shader : shaders) {
        shader.second.destroy();
    }
    shaders.clear();
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

engine::render::vk::Shader* AssetManager::setShader(engine::render::vk::Shader& shader, const string& name) {
    if(shaderExists(name)) {
        LOG(LogLevel::Warning) << "Shader asset " << name << " replaced by a new shader";
    }
    shaders[name] = shader;
    return &shaders[name];
}

engine::render::vk::Shader& AssetManager::getShader(const string& name) {
    return shaders[name];
}

engine::render::vk::Material& AssetManager::getMaterial(const string& name) {
    return materials[name];
}

void AssetManager::createMaterial(const string& name, const string& shaderName, const string& textureName) {
    renderer.createMaterial(name, shaderName, textureName);
}

engine::render::vk::Material*
engine::asset::AssetManager::setMaterial(engine::render::vk::Material& material,
                                         const string& name) {
    if(materialExists(name)) {
        LOG(LogLevel::Warning) << "Material asset " << name << " replaced by a new material";
    }
    materials[name] = material;
    return &materials[name];
}

bool engine::asset::AssetManager::shaderExists(const string& name) const {
    auto it = shaders.find(name);
    if (it != shaders.end()) return true;
    return false;
}

bool engine::asset::AssetManager::materialExists(const string& name) const {
    auto it = materials.find(name);
    if (it != materials.end()) return true;
    return false;
}

