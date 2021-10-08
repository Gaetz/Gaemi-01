//
// Created by gaetz on 08/10/2021.
//

#ifndef RENDERER_VK_RENDERERBACKENDVULKAN_H
#define RENDERER_VK_RENDERERBACKENDVULKAN_H

#include <string>
using std::string;

#include "../RendererBackEnd.h"
namespace engine::render {

    class RendererBackEndVulkan : public RendererBackEnd {
    public:
        ~RendererBackEndVulkan() override = default;

        bool init(const string& appName) override;

        void close() override;

        bool beginFrame(u32 dt) override;

        bool endFrame(u32 dt) override;
    };

}

#endif //RENDERER_VK_RENDERERBACKENDVULKAN_H
