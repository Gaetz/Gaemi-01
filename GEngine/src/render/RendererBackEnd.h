//
// Created by gaetz on 08/10/2021.
//

#ifndef RENDERER_RENDERERBACKEND_H
#define RENDERER_RENDERERBACKEND_H

#include "Types.h"
#include "../platforms/Platform.h"
#include "../math/Types.h"

// Buffer this number of frames when rendering
constexpr u32 FRAME_OVERLAP = 2;
constexpr u32 MAX_OBJECTS = 10000;

using namespace engine::math;

namespace engine::render {

    namespace vk {
        class Texture;
        class Mesh;
        class GameObject;
        class Shader;
    }

    class RendererBackEnd {
    public:
        virtual ~RendererBackEnd() = default;

        virtual bool init(const string &appName, u16 width, u16 height) = 0;

        virtual void close() = 0;

        virtual bool beginFrame(u32 dt) = 0;

        virtual void updateGlobalState(Mat4 projection, Mat4 view) = 0;

        virtual void draw() = 0;

        virtual bool endFrame(u32 dt) = 0;

        virtual void resize() = 0;

        virtual void uploadMesh(vk::Mesh &mesh) = 0;

        virtual void addToScene(vk::GameObject& gameObject) = 0;

        virtual void waitIdle() = 0;

        virtual vk::Texture loadTexture(const string& path) = 0;

        virtual void createMaterial(const string& name, const string& shaderName, const string& textureName) = 0;

        void incrementFrame() { ++frameNumber; }

    protected:
        engine::platforms::Platform *platform {nullptr};
        u64 frameNumber{0};
    };
}

#endif //RENDERER_RENDERERBACKEND_H
