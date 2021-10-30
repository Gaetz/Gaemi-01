//
// Created by gaetz on 30/10/2021.
//

#ifndef ENGINE_ASSET_ASSETMANAGER_H
#define ENGINE_ASSET_ASSETMANAGER_H

#include <map>
#include "Assets.h"

using std::map;

namespace engine::render {
    class RendererFrontEnd;
}

namespace engine::asset {

    class AssetManager : public Assets {
    public:
        explicit AssetManager(render::RendererFrontEnd& renderer);

        bool init();

        // Retrieves a stored texture
        render::vk::Texture& getTexture(const string& name) override;

        // Loads (and generates) a texture from file
        void loadTexture(const string& file, const string& name) override;

        // Retrieves a stored mesh
        render::vk::Mesh& getMesh(const string& name) override;

        // Loads (and generates) a texture from file
        void loadMesh(const string& file, const string& name) override;

        // Create a material
        void createMaterial(VkPipeline pipelineP, VkPipelineLayout pipelineLayoutP, const string& name) override;

        // Retrieves a stored material
        engine::render::vk::Material& getMaterial(const string& name) override;

        // Properly de-allocates all loaded resources
        void close();

    private:
        // Store vulkan textures
        map<string, render::vk::Texture> textures;
        // Store vulkan meshes
        map<string, render::vk::Mesh> meshes;
        // Store vulkan materials
        map<string, render::vk::Material> materials;

        render::RendererFrontEnd& renderer;
    };
}


#endif //ENGINE_ASSETS_ASSETMANAGER_H
