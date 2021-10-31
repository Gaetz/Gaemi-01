//
// Created by gaetz on 08/10/2021.
//

#ifndef RENDERER_RENDERERFRONTEND_H
#define RENDERER_RENDERERFRONTEND_H

#include "Types.h"
#include "RendererBackEnd.h"
#include "vk/Texture.h"
#include "vk/Mesh.h"
#include "vk/GameObject.h"

using std::string;

namespace engine::render {

    class RendererFrontEnd {
    public:
        RendererFrontEnd() = default;
        ~RendererFrontEnd() = default;

        bool init(const string &appName, u16 width, u16 height);
        void close();
        void onRezise(u16 width, u16 height);
        bool beginFrame(u32 dt);
        bool endFrame(u32 dt);
        bool drawFrame(const engine::render::RenderPacket &packet);

        vk::Texture loadTexture(const string& path);
        void upload(vk::Mesh& mesh);
        void addToScene(engine::render::vk::GameObject &gameObject);
        void waitIdle();

    private:
        RendererBackEnd *backEnd;
    };
}

#endif //RENDERER_RENDERERFRONTEND_H
