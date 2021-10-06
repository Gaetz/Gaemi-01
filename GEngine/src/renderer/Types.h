//
// Created by gaetz on 06/10/2021.
//

#ifndef RENDERER_TYPES_H
#define RENDERER_TYPES_H

#include "../Defines.h"

namespace engine::renderer {

    enum class RendererBackendType {
        Vulkan,
        OpenGL,
        DirectX12,
        DirectX11
    };

    class RendererBackend {
    public:

    private:
        Platform* platform;
    };

}


#endif //RENDERER_TYPES_H
