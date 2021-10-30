//
// Created by gaetz on 30/10/2021.
//

#ifndef ENGINE_ASSET_ASSETS_H
#define ENGINE_ASSET_ASSETS_H

#include <string>
#include "../Defines.h"
#include <vulkan/vulkan_core.h>

using std::string;

namespace engine::render::vk {
    class Texture;

    class Mesh;

    class Material;

    class Context;
}

namespace engine::asset {

    class Assets {
    public:
        virtual ~Assets() = default;

        // Retrieves a stored Vulkan texture
        GAPI virtual engine::render::vk::Texture& getTexture(const string& name) = 0;

        // Loads (and generates) a Vulkan texture from file
        GAPI virtual void loadTexture(const string& file, const string& name) = 0;

        // Retrieves a stored Vulkan texture
        GAPI virtual engine::render::vk::Mesh& getMesh(const string& name) = 0;

        // Loads (and generates) a Vulkan texture from file
        GAPI virtual void loadMesh(const string& file, const string& name) = 0;

        GAPI virtual engine::render::vk::Material& getMaterial(const string& name) = 0;

        virtual void
        createMaterial(VkPipeline pipelineP, VkPipelineLayout pipelineLayoutP, const string& name) = 0;
    };

    class NullAssets : public Assets {
    public:
        engine::render::vk::Texture& getTexture(const string& name) override;

        void loadTexture(const string& file, const string& name) override;

        engine::render::vk::Mesh& getMesh(const string& name) override;

        void loadMesh(const string& file, const string& name) override;

        engine::render::vk::Material& getMaterial(const string& name) override;

        void createMaterial(VkPipeline pipelineP, VkPipelineLayout pipelineLayoutP, const string& name) override;

    private:
        void placeholderMessage();
    };

}

#endif //ENGINE_ASSET_ASSETS_H
