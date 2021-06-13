//
// Created by gaetz on 13/06/2021.
//

#ifndef VK_MESH_H
#define VK_MESH_H

#include <vector>
#include "../math/Types.h"
#include "Types.h"

using std::vector;
using engine::vk::AllocatedBuffer;
using engine::math::Vec3;

namespace engine::vk {

struct VertexInputDescription {
    vector<VkVertexInputBindingDescription> bindings;
    vector<VkVertexInputAttributeDescription> attributes;
};

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec3 color;

    static VertexInputDescription getVertexDescription();
};

struct Mesh {
    vector<Vertex> vertices;
    AllocatedBuffer vertexBuffer;
};

}
#endif //VK_MESH_H
