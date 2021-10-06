//
// Created by gaetz on 13/06/2021.
//
#define TINYOBJLOADER_IMPLEMENTATION

#include <tiny_obj_loader.h>
#include "Mesh.h"
#include "../../Log.h"

using engine::renderer::vk::VertexInputDescription;
using engine::renderer::vk::Vertex;
using engine::renderer::vk::Mesh;

VertexInputDescription Vertex::getVertexDescription() {
    VertexInputDescription description;

    // We will have just 1 vertex buffer binding, with a per-vertex rate
    VkVertexInputBindingDescription mainBinding {};
    mainBinding.binding = 0;
    mainBinding.stride = sizeof(Vertex);
    mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    description.bindings.push_back(mainBinding);

    // Position will be stored at Location 0
    VkVertexInputAttributeDescription positionAttribute {};
    positionAttribute.binding = 0;
    positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = offsetof(Vertex, position);

    // Normal will be stored at Location 1
    VkVertexInputAttributeDescription normalAttribute {};
    normalAttribute.binding = 0;
    normalAttribute.location = 1;
    normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normalAttribute.offset = offsetof(Vertex, normal);

    // Color will be stored at Location 2
    VkVertexInputAttributeDescription colorAttribute {};
    colorAttribute.binding = 0;
    colorAttribute.location = 2;
    colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    colorAttribute.offset = offsetof(Vertex, color);

    // Color will be stored at Location 3
    VkVertexInputAttributeDescription uvAttribute {};
    uvAttribute.binding = 0;
    uvAttribute.location = 3;
    uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    uvAttribute.offset = offsetof(Vertex, uv);

    description.attributes.push_back(positionAttribute);
    description.attributes.push_back(normalAttribute);
    description.attributes.push_back(colorAttribute);
    description.attributes.push_back(uvAttribute);
    return description;
}

bool Mesh::loadFromObj(const string& filename) {
    tinyobj::ObjReaderConfig readerConfig;
    //readerConfig.mtl_search_path = "./"; // Path to material files
    tinyobj::ObjReader reader;

    // Read file and handle errors and warnings
    if (!reader.ParseFromFile(filename, readerConfig)) {
        if (!reader.Error().empty()) {
            LOG(LogLevel::Error) << "TinyObjReader: " << reader.Error();
        }
        return false;
    }
    if (!reader.Warning().empty()) {
        LOG(LogLevel::Warning) << "TinyObjReader: " << reader.Warning();
    }

    // Populate mesh
    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // Loop over shapes
    for (const auto & shape : shapes) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {

            // Hardcode faces to 3
            size_t fv = 3;
            //size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // Access to vertex
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                // Vertex position
                tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
                tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
                tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];

                // Check if `normal_index` is zero or positive. negative = no normal data
                tinyobj::real_t nx = 0, ny = 0, nz = 0;
                if (idx.normal_index >= 0) {
                    nx = attrib.normals[3*size_t(idx.normal_index)+0];
                    ny = attrib.normals[3*size_t(idx.normal_index)+1];
                    nz = attrib.normals[3*size_t(idx.normal_index)+2];
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
                    tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
                }

                // Optional: vertex colors
                // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

                // uv
                tinyobj::real_t ux = attrib.texcoords[2 * idx.texcoord_index + 0];
                tinyobj::real_t uy = attrib.texcoords[2 * idx.texcoord_index + 1];

                // Create vertex
                Vertex newVertex;
                newVertex.position.x = vx;
                newVertex.position.y = vy;
                newVertex.position.z = vz;

                newVertex.normal.x = nx;
                newVertex.normal.y = ny;
                newVertex.normal.z = nz;

                newVertex.uv.x = ux;
                newVertex.uv.y = 1 - uy;

                // If you want to see normals
                //newVertex.color = newVertex.normal;

                vertices.push_back(newVertex);
            }
            index_offset += fv;

            // per-face material
            //shape.mesh.material_ids[f];
        }
    }
    LOG(LogLevel::Info) << "TinyObjReader: successfully loaded " << filename << reader.Error();
    return true;
}
