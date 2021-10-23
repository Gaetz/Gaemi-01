//
// Created by gaetz on 12/06/2021.
//

#ifndef RENDER_VK_DELETIONQUEUE_H
#define RENDER_VK_DELETIONQUEUE_H

#include <functional>
#include <deque>

namespace engine::render::vk {
    struct DeletionQueue {
        std::deque<std::function<void()>> deletors;

        void pushFunction(std::function<void()>&& function) {
            deletors.push_back(function);
        }

        void flush() {
            // Reverse iterate the deletion queue to execute all the functions
            for (auto it = deletors.rbegin(); it != deletors.rend(); ++it) {
                (*it)(); // Call the function
            }
            deletors.clear();
        }
    };
}


#endif //RENDER_VK_DELETIONQUEUE_H
