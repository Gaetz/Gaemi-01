//
// Created by gaetz on 30/10/2021.
//

#ifndef RENDER_VK_TEXTURE_H
#define RENDER_VK_TEXTURE_H

#include "Types.h"

namespace engine::render::vk {

    class Texture {
    public:
        AllocatedImage image;
        VkImageView imageView;
        VkSampler sampler;
    };
}
#endif //RENDER_VK_TEXTURE_H
