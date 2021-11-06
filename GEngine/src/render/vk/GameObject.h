//
// Created by gaetz on 15/06/2021.
//

#ifndef RENDER_VK_RENDEROBJECT_H
#define RENDER_VK_RENDEROBJECT_H

#include "Mesh.h"
#include "Material.h"

namespace engine::render::vk {

    struct GameObject {
        Mesh *mesh;
        Material *material;
        Mat4 transform;
    };

}

#endif //RENDER_VK_RENDEROBJECT_H
