//
// Created by gaetz on 06/10/2021.
//

#ifndef MEMORYTAG_H
#define MEMORYTAG_H

namespace engine::mem {

    enum class MemoryTag {
        Unknown,
        Array,
        DArray,
        Dict,
        RingQueue,
        Bst,
        String,
        Application,
        Job,
        Texture,
        MaterialInstance,
        Renderer,
        Game,
        Transform,
        Entity,
        EntityNode,
        Scene,

        MaxMemoryTag
    };

    constexpr u8 MEMORY_TAG_MAX = static_cast<u8>(MemoryTag::MaxMemoryTag);
}

#endif //MEMORYTAG_H
