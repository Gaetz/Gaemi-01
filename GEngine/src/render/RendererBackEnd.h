//
// Created by gaetz on 08/10/2021.
//

#ifndef RENDERER_RENDERERBACKEND_H
#define RENDERER_RENDERERBACKEND_H

#include "Types.h"
#include "../platforms/Platform.h"

namespace engine::render {

    class RendererBackEnd {
    public:
        virtual ~RendererBackEnd() {};

        virtual bool init(const string& appName) = 0;

        virtual void close() = 0;

        virtual bool beginFrame(u32 dt) = 0;

        virtual bool endFrame(u32 dt) = 0;

        void incrementFrame() { ++frameNumber; }

    private:
        engine::platforms::Platform *platform{nullptr};
        u64 frameNumber{0};
    };
}

#endif //RENDERER_RENDERERBACKEND_H
