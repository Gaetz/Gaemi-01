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

    class Shader;
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

        // Retrieves a stored Vulkan shader
        GAPI virtual engine::render::vk::Shader& getShader(const string& name) = 0;

        // Push a shader into the asset manager.
        // Shader is loaded through the createMaterial method.
        virtual engine::render::vk::Shader* setShader(engine::render::vk::Shader&& shader, const string& name) = 0;

        GAPI virtual engine::render::vk::Material& getMaterial(const string& name) = 0;

        virtual engine::render::vk::Material* setMaterial(render::vk::Material& material, const string& name) = 0;

        // Create a material with a texture
        GAPI virtual void createMaterial(const string& name, const string& textureName) = 0;
    };


    class NullAssets : public Assets {
    public:
        engine::render::vk::Texture& getTexture(const string& name) override;

        void loadTexture(const string& file, const string& name) override;

        engine::render::vk::Mesh& getMesh(const string& name) override;

        void loadMesh(const string& file, const string& name) override;

        engine::render::vk::Shader& getShader(const string& name) override;

        engine::render::vk::Shader* setShader(engine::render::vk::Shader&& shader, const string& name) override;

        engine::render::vk::Material& getMaterial(const string& name) override;

        engine::render::vk::Material* setMaterial(render::vk::Material& material, const string& name) override;

        void createMaterial(const string& name, const string& textureName) override;


    private:
        void placeholderMessage();
    };

}

#endif //ENGINE_ASSET_ASSETS_H
