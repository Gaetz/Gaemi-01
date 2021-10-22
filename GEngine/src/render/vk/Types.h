//
// Created by gaetz on 08/06/2021.
//

#ifndef VK_TYPES_H
#define VK_TYPES_H

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "../../math/Types.h"
#include "DeletionQueue.h"

using engine::math::Vec4;
using engine::math::Mat4;

#ifdef _DEBUG
#define VK_CHECK(x)                                                             \
    do                                                                          \
    {                                                                           \
        VkResult err = x;                                                       \
        if (err)                                                                \
        {                                                                       \
            LOG(LogLevel::Error) << "Detected Vulkan error: " << err;           \
            abort();                                                            \
        }                                                                       \
    } while (0)
#else
#define VK_CHECK(x) \
        x
#endif

#ifdef _DEBUG
#define VK_SWAP_CHECK(x)                                                        \
        VkResult res = x;                                                       \
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {      \
            reinit();                                                           \
        } else if (res != VK_SUCCESS) {                                          \
            LOG(LogLevel::Error) << "Detected Swapchain error: " << res;        \
            abort();                                                                                                                                     \
        }
#else
#define VK_CHECK(x)                                                             \
        VkResult res = x;                                                       \
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {      \
            reinit();                                                           \
        }
#endif

namespace engine::render::vk {

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct AllocatedImage {
    VkImage image;
    VmaAllocation allocation;
};

struct MeshPushConstants {
    Vec4 data;
    Mat4 renderMatrix;
};

struct FrameData {
    VkSemaphore presentSemaphore, renderSemaphore;
    VkFence renderFence;

    VkCommandPool commandPool;
    VkCommandBuffer mainCommandBuffer;
    AllocatedBuffer cameraBuffer;
    VkDescriptorSet globalDescriptor;

    AllocatedBuffer objectBuffer;
    VkDescriptorSet objectDescriptor;
};

struct GPUCameraData {
    Mat4 view;
    Mat4 projection;
    Mat4 viewProj;
};

struct GPUSceneData {
	Vec4 fogColor; // w is for exponent
	Vec4 fogDistances; // x for min, y for max, zw unused.
	Vec4 ambientColor;
	Vec4 sunlightDirection; // w for sun power
	Vec4 sunlightColor;
};

struct GPUObjectData {
    Mat4 modelMatrix;
};

struct UploadContext {
    VkFence uploadFence;
    VkCommandPool commandPool;
};

class Texture {
public:
    AllocatedImage image;
    VkImageView imageView;
};
}

#endif //VK_TYPES_H
