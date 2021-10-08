//
// Created by gaetz on 15/06/2021.
//

#ifndef VK_RENDEROBJECT_H
#define VK_RENDEROBJECT_H

#include "Mesh.h"

namespace engine::render::vk {

struct Material {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet textureSet { VK_NULL_HANDLE };
};

struct RenderObject {
    Mesh* mesh;
    Material* material;
    Mat4 transform;
};

}

#endif //VK_RENDEROBJECT_H
