//
// Created by gaetz on 13/06/2021.
//

#ifndef VK_MESH_H
#define VK_MESH_H

#include <vector>
#include <string>

#include "Types.h"

using std::string;
using std::vector;
using engine::render::vk::AllocatedBuffer;
using engine::math::Vec3;
using engine::math::Vec2;

namespace engine::render::vk {

struct VertexInputDescription {
    vector<VkVertexInputBindingDescription> bindings;
    vector<VkVertexInputAttributeDescription> attributes;
};

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec3 color;
    Vec2 uv;
    static VertexInputDescription getVertexDescription();
};

struct Mesh {
    vector<Vertex> vertices;
    AllocatedBuffer vertexBuffer;

    bool loadFromObj(const string& filename);
};

}
#endif //VK_MESH_H