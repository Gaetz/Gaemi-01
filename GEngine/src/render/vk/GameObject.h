//
// Created by gaetz on 15/06/2021.
//

#ifndef RENDER_VK_RENDEROBJECT_H
#define RENDER_VK_RENDEROBJECT_H

#include "Mesh.h"
#include "Shader.h"

namespace engine::render::vk {

    struct Material {
        Shader* shader { nullptr };
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        VkDescriptorSet textureSet { VK_NULL_HANDLE };
    };

    struct GameObject {
        Mesh *mesh;
        Material *material;
        Mat4 transform;
    };

}

#endif //RENDER_VK_RENDEROBJECT_H
