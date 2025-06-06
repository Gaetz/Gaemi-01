//
// Created by gaetz on 08/10/2021.
//

#include "RendererBackEndVulkan.h"

#define VMA_IMPLEMENTATION

#include <vma/vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION

#include <../../externals/stb_image/stb_image.h>

#include <SDL2/SDL_vulkan.h>

#include "../../externals/vkbootstrap/VkBootstrap.h"
#include "Init.h"
#include "../../math/Transformations.h"
#include "../../math/Functions.h"
#include "../../Locator.h"
#include "Buffer.h"
#include "../../../../externals/imgui/imgui_impl_vulkan.h"

using engine::render::vk::PipelineBuilder;
using engine::render::vk::Vertex;
using engine::render::vk::RendererBackEndVulkan;
using engine::render::vk::Texture;

bool RendererBackEndVulkan::init(const string& appName, u16 width, u16 height) {
    context.windowExtent.width = width;
    context.windowExtent.height = height;

    context.init(appName);
    swapchain.init();
    initCommands();
    renderpass.init(swapchain);
    regenerateFramebuffers();
    initSyncStructures();
    initDescriptors();
    initImgui();
    loadDefaultAssets();
    return true;
}

void RendererBackEndVulkan::close() {
    cleanupVulkan();
}

void RendererBackEndVulkan::resize() {

}

void RendererBackEndVulkan::cleanupVulkan() {
    // Make sure the GPU has stopped before clearing
    --frameNumber;
    getCurrentFrame().renderFence.wait(context, 1000000000);
    ++frameNumber;

    context.close();
}

void RendererBackEndVulkan::initCommands() {
    // Command pool
    auto commandPoolInfo = render::vk::commandPoolCreateInfo(context.graphicsQueueFamily,
                                                             VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (int i = 0; i < FRAME_OVERLAP; ++i) {
        FrameData& frame = frames[i];
        VK_CHECK(vkCreateCommandPool(context.device, &commandPoolInfo, nullptr, &frame.commandPool));

        // Command buffer
        frame.mainCommandBuffer.allocate(context, frame.commandPool, true, false);

        // Cleanup callback
        context.mainDeletionQueue.pushFunction([=]() {
            vkDestroyCommandPool(context.device, frame.commandPool, nullptr);
        });
    }

    // Upload command pool
    auto uploadCommandPoolInfo = render::vk::commandPoolCreateInfo(context.graphicsQueueFamily);
    VK_CHECK(vkCreateCommandPool(context.device, &uploadCommandPoolInfo, nullptr, &uploadContext.commandPool));
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyCommandPool(context.device, uploadContext.commandPool, nullptr);
    });
}

void RendererBackEndVulkan::regenerateFramebuffers() {
    // Grab how many images we have in the swapchain
    const uint32_t swapchainImageCount = swapchain.images.size();
    framebuffers.reserve(swapchainImageCount);

    // Create framebuffers for each of the swapchain image views
    for (size_t i = 0; i < swapchainImageCount; ++i) {
        // Create framebuffer
        framebuffers.emplace_back(context, renderpass);
        framebuffers[i].init(swapchain.imageViews[i], swapchain.depthImageView);
    }
}

void RendererBackEndVulkan::initSyncStructures() {
    // Image fence and semaphores
    for (int i = 0; i < FRAME_OVERLAP; ++i) {
        frames[i].renderFence.init(context, true);
        initSemaphore(context, frames[i].presentSemaphore);
        initSemaphore(context, frames[i].renderSemaphore);
    }

    // Upload fence
    uploadContext.uploadFence.init(context, false);
}


bool RendererBackEndVulkan::beginFrame(u32 dt) {
    // Wait until the GPU has finished rendering the last frame. Timeout of 1 second
    getCurrentFrame().renderFence.wait(context, 1000000000);
    getCurrentFrame().renderFence.reset(context);

    // Request image from the swapchain, one second timeout
    swapchain.acquireNextImage(1000000000, getCurrentFrame().presentSemaphore, nullptr, swapchain.imageIndex);

    // Naming it cmd for shorter writing
    CommandBuffer cmd = getCurrentFrame().mainCommandBuffer;

    // Now that we are sure that the commands finished executing,
    // we can safely reset the command buffer to begin recording again.
    cmd.reset();

    // Begin the command buffer recording. We will use this command buffer exactly once,
    // so we want to let Vulkan know that
    cmd.begin(true, false, false);

    renderpass.begin(cmd, framebuffers[swapchain.imageIndex]);
    return true;
}

void RendererBackEndVulkan::draw() {
    // -- DRAW HERE --
    CommandBuffer cmd = getCurrentFrame().mainCommandBuffer;
    drawObjects(cmd, renderables.data(), renderables.size());
}

bool RendererBackEndVulkan::endFrame(u32 dt) {
    CommandBuffer cmd = getCurrentFrame().mainCommandBuffer;

    // Finalize the render pass
    renderpass.end(cmd);
    // Finalize the command buffer (we can no longer add commands, but it can now be executed)
    cmd.end();

    // Prepare the submission to the queue.
    // We want to wait on the presentSemaphore, as that semaphore is isSignaled when the swapchain is ready.
    // We will signal the renderSemaphore, to signal that rendering has finished.
    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &getCurrentFrame().presentSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &getCurrentFrame().renderSemaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd.handle;

    // Submit command buffer to the queue and execute it.
    // renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, getCurrentFrame().renderFence.handle));
    cmd.updateSubmitted();

    // Present the rendered image into the visible window
    swapchain.present(getCurrentFrame().renderSemaphore, swapchain.imageIndex);
    return true;
}

AllocatedBuffer
RendererBackEndVulkan::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo {};
    vmaAllocInfo.usage = memoryUsage;

    // Allocate memory
    AllocatedBuffer allocatedBuffer {};
    VK_CHECK(vmaCreateBuffer(context.allocator,
                             &bufferInfo,
                             &vmaAllocInfo,
                             &allocatedBuffer.buffer,
                             &allocatedBuffer.allocation,
                             nullptr));

    // Cleanup
    context.mainDeletionQueue.pushFunction([=]() {
        vmaDestroyBuffer(context.allocator, allocatedBuffer.buffer, allocatedBuffer.allocation);
    });


    return allocatedBuffer;
}


size_t RendererBackEndVulkan::padUniformBufferSize(size_t originalSize) const {
    // Calculate required alignment based on minimum device offset alignment
    size_t minUboAlignment = context.gpuProperties.limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = originalSize;
    if (minUboAlignment > 0) {
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    return alignedSize;
}


void RendererBackEndVulkan::initDescriptors() {
    // Create a descriptor pool that will hold 10 uniform buffers
    vector<VkDescriptorPoolSize> sizes {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         10 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         10 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
    };
    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = 0;
    poolInfo.maxSets = 10;
    poolInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
    poolInfo.pPoolSizes = sizes.data();
    VK_CHECK(vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &descriptorPool));
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyDescriptorPool(context.device, descriptorPool, nullptr);
    });

    // -- UNIFORM BUFFERS for camera and scene --

    // Binding for camera data at 0
    VkDescriptorSetLayoutBinding cameraBufferBinding = render::vk::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            0);
    // Binding for scene data at 1
    VkDescriptorSetLayoutBinding sceneBufferBinding = render::vk::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            1);

    array<VkDescriptorSetLayoutBinding, 2> bindings { cameraBufferBinding, sceneBufferBinding };

    // Set layout info
    VkDescriptorSetLayoutCreateInfo setInfo {};
    setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setInfo.pNext = nullptr;
    setInfo.flags = 0;
    setInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    setInfo.pBindings = bindings.data();
    VK_CHECK(vkCreateDescriptorSetLayout(context.device, &setInfo, nullptr, &globalSetLayout));
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyDescriptorSetLayout(context.device, globalSetLayout, nullptr);
    });

    // Uniform buffer for scene data: two scene params (one for each frame) in same buffer
    const size_t sceneParamBufferSize = FRAME_OVERLAP * padUniformBufferSize(sizeof(render::vk::GPUSceneData));
    sceneParamsBuffer = createBuffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VMA_MEMORY_USAGE_CPU_TO_GPU);

    // -- STORAGE BUFFER for objects --
    VkDescriptorSetLayoutBinding objectBind = render::vk::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            0);
    VkDescriptorSetLayoutCreateInfo set2Info;
    set2Info.bindingCount = 1;
    set2Info.flags = 0;
    set2Info.pNext = nullptr;
    set2Info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set2Info.pBindings = &objectBind;
    VK_CHECK(vkCreateDescriptorSetLayout(context.device, &set2Info, nullptr, &objectSetLayout));
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyDescriptorSetLayout(context.device, objectSetLayout, nullptr);
    });

    // -- SAMPLER for single texture --
    VkDescriptorSetLayoutBinding textureBind = render::vk::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0);
    VkDescriptorSetLayoutCreateInfo set3info {};
    set3info.bindingCount = 1;
    set3info.flags = 0;
    set3info.pNext = nullptr;
    set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set3info.pBindings = &textureBind;
    VK_CHECK(vkCreateDescriptorSetLayout(context.device, &set3info, nullptr, &singleTextureSetLayout));
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyDescriptorSetLayout(context.device, singleTextureSetLayout, nullptr);
    });

    // -- CREATE BUFFERS FOR EACH FRAMES --
    for (int i = 0; i < FRAME_OVERLAP; ++i) {
        frames[i].cameraBuffer = createBuffer(sizeof(render::vk::GPUCameraData),
                                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                              VMA_MEMORY_USAGE_CPU_TO_GPU);

        frames[i].objectBuffer = createBuffer(
                sizeof(render::vk::GPUObjectData) * MAX_OBJECTS,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU);

        // Allocate one descriptor for each frame, global set layout
        VkDescriptorSetAllocateInfo allocInfo {};
        allocInfo.pNext = nullptr;
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorSetCount = 1;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &globalSetLayout;
        VK_CHECK(vkAllocateDescriptorSets(context.device, &allocInfo, &frames[i].globalDescriptor));

        // Object set layout
        VkDescriptorSetAllocateInfo objectSetAllocInfo {};
        objectSetAllocInfo.pNext = nullptr;
        objectSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        objectSetAllocInfo.descriptorSetCount = 1;
        objectSetAllocInfo.descriptorPool = descriptorPool;
        objectSetAllocInfo.pSetLayouts = &objectSetLayout;
        VK_CHECK(vkAllocateDescriptorSets(context.device, &objectSetAllocInfo, &frames[i].objectDescriptor));

        // Make the descriptor point into the camera buffer
        VkDescriptorBufferInfo cameraInfo {};
        // Descriptor will point to the camera buffer
        cameraInfo.buffer = frames[i].cameraBuffer.buffer;
        // ...at 0 offset
        cameraInfo.offset = 0;
        // ...of the size of camera data struct
        cameraInfo.range = sizeof(render::vk::GPUCameraData);

        // Make the descriptor point into the scene buffer
        VkDescriptorBufferInfo sceneInfo {};
        sceneInfo.buffer = sceneParamsBuffer.buffer;
        sceneInfo.offset = 0; // For non dynamic buffer, would be: padUniformBufferSize(sizeof(render::vk::GPUSceneData)) * i;
        sceneInfo.range = sizeof(render::vk::GPUSceneData);

        // Make the object descriptor point to the object storage buffer
        VkDescriptorBufferInfo objectBufferInfo {};
        objectBufferInfo.buffer = frames[i].objectBuffer.buffer;
        objectBufferInfo.offset = 0;
        objectBufferInfo.range = sizeof(render::vk::GPUObjectData) * MAX_OBJECTS;

        // Writes
        VkWriteDescriptorSet cameraWrite = render::vk::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                             frames[i].globalDescriptor, &cameraInfo,
                                                                             0);
        VkWriteDescriptorSet sceneWrite = render::vk::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                                                            frames[i].globalDescriptor, &sceneInfo, 1);
        VkWriteDescriptorSet objectWrite = render::vk::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                                             frames[i].objectDescriptor,
                                                                             &objectBufferInfo, 0);
        array<VkWriteDescriptorSet, 3> setWrites { cameraWrite, sceneWrite, objectWrite };

        vkUpdateDescriptorSets(context.device, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
    }
}

void RendererBackEndVulkan::loadDefaultAssets() {

    /*
// Load triangle
Mesh triangleMesh;
triangleMesh.vertices.resize(3);
triangleMesh.vertices[0].position = { 1.f, 1.f, 0.f };
triangleMesh.vertices[1].position = { -1.f, 1.f, 0.f };
triangleMesh.vertices[2].position = { 0.f, -1.f, 0.f };
triangleMesh.vertices[0].color = { 0.f, 1.f, 0.f };
triangleMesh.vertices[1].color = { 0.f, 1.f, 0.f };
triangleMesh.vertices[2].color = { 0.f, 1.f, 0.f };

// Load monkey from obj file
Mesh monkeyMesh;
monkeyMesh.loadFromObj("../../assets/knot.obj");

// Lost empire
Mesh lostEmpire;
lostEmpire.loadFromObj("../../assets/lost_empire.obj");

uploadMesh(triangleMesh);
uploadMesh(monkeyMesh);
uploadMesh(lostEmpire);

// Store (copy) meshes in lists
meshes["triangle"] = triangleMesh;
meshes["monkey"] = monkeyMesh;
meshes["lostEmpire"] = lostEmpire;
 */

    // Our scene will be composed of a monkey and triangles

    /*
    GameObject monkey {};
    monkey.mesh = getMesh("monkey");
    monkey.material = getMaterial("defaultMesh");
    monkey.transform = math::translate(Mat4{1.0f}, Vec3{0, 0, -20});
    renderables.push_back(monkey);
*/
    /*
    // Monkey
    GameObject monkey {};
    monkey.mesh = getMesh("monkey");
    monkey.material = getMaterial("defaultMesh");
    monkey.transform = math::translate(Mat4{1.0f}, Vec3{0, 0, -20});
    renderables.push_back(monkey);
     */
/*
    // Triangles
    for (int x = -20; x <= 20; ++x) {
        for (int y = -20; y <= 20; ++y) {
            GameObject triangle {};
            triangle.mesh = getMesh("triangle");
            triangle.material = getMaterial("defaultMesh");
            Mat4 translation = math::translate(Mat4{1.0f}, Vec3{x, 0, y});
            Mat4 scale = math::scale(Mat4{1.0f}, Vec3{0.2f, 0.2f, 0.2f});
            triangle.transform = translation * scale;
            renderables.push_back(triangle);
        }
    }

    // Add lost empire
    GameObject map;
    map.mesh = getMesh("lostEmpire");
    map.material = getMaterial("defaultMesh");
    map.transform = math::translate(Mat4{1.f}, Vec3{ 5,-10,0 });
    renderables.push_back(map);
*/

    Locator::assets().loadTexture("../../assets/default.png", "default");
    createMaterial("default", "default", "default");
}

void RendererBackEndVulkan::uploadMesh(Mesh& mesh) {
    const size_t bufferSize = mesh.vertices.size() * sizeof(Vertex);

    // -- STAGING BUFFER to load mesh into CPU memory --
    Buffer stagingBuffer;
    stagingBuffer.init(context, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, bufferSize);
    stagingBuffer.loadData(mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));

    // -- VERTEX BUFFER, copy from staging to vertex buffer --
    mesh.vertexBuffer.init(context,
                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                           VMA_MEMORY_USAGE_GPU_ONLY, bufferSize);
    stagingBuffer.copyTo(mesh.vertexBuffer.handle, 0, 0, bufferSize,
                         uploadContext.commandPool, uploadContext.uploadFence.handle, context.graphicsQueue);
}

void RendererBackEndVulkan::drawObjects(CommandBuffer& commandBuffer, GameObject* first, size_t count) {
    VkCommandBuffer cmd = commandBuffer.handle;

    // View and projection
    Vec3 camPos { 0.f, -6.f, -10.f };
    Mat4 view = math::translate(Mat4 { 1.f }, camPos);
    Mat4 projection = math::perspective(math::toRad(70.f),
                                        static_cast<float>(context.windowExtent.width) /
                                        static_cast<float>(context.windowExtent.height),
                                        0.1f, 200.f);
    projection[1][1] *= -1; // Inverse vertical axis for Vulkan

    // Fill GPUCamera data struct and copy it to the buffer
    render::vk::GPUCameraData cameraData {};
    cameraData.projection = projection;
    cameraData.view = view;
    cameraData.viewProj = projection * view;
    void* data;
    vmaMapMemory(context.allocator, getCurrentFrame().cameraBuffer.allocation, &data);
    memcpy(data, &cameraData, sizeof(render::vk::GPUCameraData));
    vmaUnmapMemory(context.allocator, getCurrentFrame().cameraBuffer.allocation);

    // Fill GPUSceneData with random ambient color
    float framed = static_cast<float>(frameNumber) / 120.0f;
    sceneParams.ambientColor = { math::sin(framed), 0, math::cos(framed), 1 };

    char* sceneData;
    vmaMapMemory(context.allocator, sceneParamsBuffer.allocation, (void**) &sceneData);
    unsigned int frameIndex = frameNumber % FRAME_OVERLAP;
    sceneData += padUniformBufferSize(sizeof(render::vk::GPUSceneData)) * frameIndex;
    memcpy(sceneData, &sceneParams, sizeof(render::vk::GPUSceneData));
    vmaUnmapMemory(context.allocator, sceneParamsBuffer.allocation);

    // Fill storage buffer with object model matrices
    void* objectData;
    vmaMapMemory(context.allocator, getCurrentFrame().objectBuffer.allocation, &objectData);
    render::vk::GPUObjectData* objectSSBO = (render::vk::GPUObjectData*) objectData;
    for (int i = 0; i < count; ++i) {
        GameObject& object = first[i];
        objectSSBO[i].modelMatrix = object.transform;
    }
    vmaUnmapMemory(context.allocator, getCurrentFrame().objectBuffer.allocation);

    // Draw with push constants
    Mesh* lastMesh = nullptr;
    render::vk::Material* lastMaterial = nullptr;
    for (int i = 0; i < count; ++i) {
        GameObject& object = first[i];

        // Bind pipeline if material is different
        if (object.material != lastMaterial) {
            object.material->shader->use(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
            //vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
            lastMaterial = object.material;

            // -- CAMERA DESCRIPTOR --
            // Offset for scene data dynamic buffer
            uint32_t uniformOffset = padUniformBufferSize(sizeof(render::vk::GPUSceneData)) * frameIndex;
            // Bind descriptor set when changing pipeline
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 0, 1,
                                    &getCurrentFrame().globalDescriptor, 1, &uniformOffset);

            // -- OBJECT DATA DESCRIPTOR --
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 1, 1,
                                    &getCurrentFrame().objectDescriptor, 0, nullptr);

            // -- TEXTURE --
            if (object.material->textureSet != VK_NULL_HANDLE) {
                // Texture descriptor
                vkCmdBindDescriptorSets(cmd,
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        object.material->pipelineLayout,
                                        2, 1, &object.material->textureSet,
                                        0, nullptr);
            }
        }

        // Push transform
        render::vk::MeshPushConstants constants {};
        constants.renderMatrix = object.transform;
        vkCmdPushConstants(cmd,
                           object.material->pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT,
                           0, sizeof(render::vk::MeshPushConstants),
                           &constants);

        // Bind mesh if mesh is different
        if (object.mesh != lastMesh) {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->vertexBuffer.handle, &offset);
            lastMesh = object.mesh;
        }

        // Draw
        vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, i);

        // Draw ImGui
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    }
}

void RendererBackEndVulkan::immediateSubmit(std::function<void(VkCommandBuffer)>&& submittedFunc) {

    CommandBuffer cmd;
    cmd.singleUseBegin(context, uploadContext.commandPool);

    // Execute function
    submittedFunc(cmd.handle);

    cmd.singleUseEnd(context, uploadContext.commandPool, context.graphicsQueue, uploadContext.uploadFence);

    // Clear the command pool. This will free the command buffer too
    vkResetCommandPool(context.device, uploadContext.commandPool, 0);
}

Texture RendererBackEndVulkan::loadTexture(const string& path) {
    render::vk::Texture texture {};
    loadTextureFromFile(path, texture.image);

    // Image view
    VkImageViewCreateInfo imageViewInfo = render::vk::imageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, texture.image.image,
                                                                          VK_IMAGE_ASPECT_COLOR_BIT);
    vkCreateImageView(context.device, &imageViewInfo, nullptr, &texture.imageView);
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyImageView(context.device, texture.imageView, nullptr);
    });

    // Sampler
    VkSamplerCreateInfo samplerInfo = render::vk::samplerCreateInfo(VK_FILTER_NEAREST);
    vkCreateSampler(context.device, &samplerInfo, nullptr, &texture.sampler);
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroySampler(context.device, texture.sampler, nullptr);
    });

    return texture;
}

bool RendererBackEndVulkan::loadTextureFromFile(const string& path, render::vk::AllocatedImage& outImage) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) {
        LOG(LogLevel::Error) << "Failed to load texture file " << path;
        return false;
    }
    void* pixel_ptr = pixels;
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

    Buffer stagingBuffer;
    stagingBuffer.init(context, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, imageSize);
    stagingBuffer.loadData(pixel_ptr, static_cast<size_t>(imageSize));

    stbi_image_free(pixels);

    VkExtent3D imageExtent;
    imageExtent.width = static_cast<uint32_t>(texWidth);
    imageExtent.height = static_cast<uint32_t>(texHeight);
    imageExtent.depth = 1;

    render::vk::AllocatedImage newImage {};
    VkImageCreateInfo imageCreateInfo = render::vk::imageCreateInfo(imageFormat,
                                                                    VK_IMAGE_USAGE_SAMPLED_BIT |
                                                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                                    imageExtent);
    VmaAllocationCreateInfo imageAllocInfo {};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    // Allocate and create the image
    vmaCreateImage(context.allocator, &imageCreateInfo, &imageAllocInfo, &newImage.image, &newImage.allocation,
                   nullptr);

    // Transition image to transfer-receiver
    immediateSubmit([&](VkCommandBuffer cmd) {
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;

        VkImageMemoryBarrier imageBarrierToTransfer {};
        imageBarrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierToTransfer.image = newImage.image;
        imageBarrierToTransfer.subresourceRange = range;
        imageBarrierToTransfer.srcAccessMask = 0;
        imageBarrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        // Barrier the image into the transfer-receive layout
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                             nullptr, 1, &imageBarrierToTransfer);

        VkBufferImageCopy copyRegion {};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;

        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageExtent = imageExtent;

        // Copy the buffer into the image
        vkCmdCopyBufferToImage(cmd, stagingBuffer.handle, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &copyRegion);

        VkImageMemoryBarrier imageBarrierToReadable = imageBarrierToTransfer;
        imageBarrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageBarrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierToReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // Barrier the image into the shader readable layout
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                             0, nullptr, 1, &imageBarrierToReadable);
    });

    context.mainDeletionQueue.pushFunction([=]() {
        vmaDestroyImage(context.allocator, newImage.image, newImage.allocation);
    });

    LOG(LogLevel::Info) << "Texture loaded successfully " << path;
    outImage = newImage;
    return true;
}

void RendererBackEndVulkan::addToScene(engine::render::vk::GameObject& gameObject) {
    renderables.push_back(gameObject);
}

void RendererBackEndVulkan::waitIdle() {
    context.waitIdle();
}

void RendererBackEndVulkan::createMaterial(const string& name, const string& shaderName, const string& textureName) {

    // TODO should shader with difference texture be a different material ?

    auto& assets { Locator::assets() };
    Shader* shaderAddress { nullptr };
    if (assets.shaderExists(shaderName)) {
        shaderAddress = &assets.getShader(shaderName);
    } else {
        // Set shader and store in asset manager. Shader name used to load shader files.
        Shader shader;
        array<VkDescriptorSetLayout, 3> setLayouts { globalSetLayout, objectSetLayout, singleTextureSetLayout };
        shader.init(context, setLayouts, renderpass, descriptorPool, name);
        shaderAddress = assets.setShader(shader, name);
    }

    // Set material and store in asset manager
    Material material;
    material.shader = shaderAddress;
    material.pipelineLayout = shaderAddress->pipeline.layoutHandle;
    material.pipeline = shaderAddress->pipeline.handle;
    Material* materialAddress = assets.setMaterial(material, name);

    // Texture to material

    // Allocate the descriptor set for single-texture to use on the material
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &singleTextureSetLayout;
    vkAllocateDescriptorSets(context.device, &allocInfo, &materialAddress->textureSet);

    // Write to the descriptor set so that it points to our texture <textureName>
    const Texture& texture = Locator::assets().getTexture(textureName);
    VkDescriptorImageInfo imageBufferInfo;
    imageBufferInfo.sampler = texture.sampler;;
    imageBufferInfo.imageView = texture.imageView;
    imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet texture1 = render::vk::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                                     materialAddress->textureSet, &imageBufferInfo, 0);
    vkUpdateDescriptorSets(context.device, 1, &texture1, 0, nullptr);
}

void engine::render::vk::RendererBackEndVulkan::updateGlobalState(Mat4 projection, Mat4 view) {

}

void engine::render::vk::RendererBackEndVulkan::initImgui() {
    // Create descriptor pool for IMGUI
    // The size of the pool is very oversize, but it's copied from imgui demo itself.
    VkDescriptorPoolSize poolSizes[] =
            {
                    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = std::size(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    VkDescriptorPool imguiPool;
    VK_CHECK(vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &imguiPool));


    // Initialize imgui library

    // This initializes the core structures of imgui
    ImGui::CreateContext();

    // This initializes imgui for SDL
    Locator::platform().rendererSetupVulkanImGui();

    // This initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = context.instance;
    init_info.PhysicalDevice = context.chosenGPU;
    init_info.Device = context.device;
    init_info.Queue = context.graphicsQueue;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, renderpass.handle);

    //execute a gpu command to upload imgui font textures
    immediateSubmit([&](VkCommandBuffer cmd) {
        ImGui_ImplVulkan_CreateFontsTexture(cmd);
    });

    //clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    //add the destroy the imgui created structures
    context.mainDeletionQueue.pushFunction([=]() {
        vkDestroyDescriptorPool(context.device, imguiPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
    });
}


