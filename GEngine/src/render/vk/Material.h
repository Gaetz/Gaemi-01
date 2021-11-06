//
// Created by gaetz on 06/11/2021.
//

#ifndef RENDER_VK_MATERIAL_H
#define RENDER_VK_MATERIAL_H

#include "Shader.h"

namespace engine::render::vk {

    struct Material {
        Shader* shader { nullptr };
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        VkDescriptorSet textureSet { VK_NULL_HANDLE };
    };

}

#endif //RENDER_VK_MATERIAL_H
