//
// Created by gaetz on 26/10/2021.
//

#include "Shader.h"
#include "Context.h"
#include "Renderpass.h"
#include "Init.h"
#include "Mesh.h"
#include <vector>

using engine::render::vk::Shader;
using std::vector;

void Shader::init(Context& context,
                  array<VkDescriptorSetLayout, 3> setLayouts,
                  const Renderpass& renderpass,
                  VkDescriptorPool descriptorPool,
                  const string& shaderName) {
    name = shaderName;
    contextDevice = context.device;

    // Shader module and stages
    array<string, SHADER_STAGE_COUNT> typeStrings { "vert", "frag" };
    array<VkShaderStageFlagBits, SHADER_STAGE_COUNT> types { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
    for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
        if (!load(context, shaderName, typeStrings[i], types[i], stages[i])) {
            LOG(LogLevel::Error) << "Error when building shader " << shaderName << " at stage " << i;
        } else {
            LOG(LogLevel::Info) << "Shader " << shaderName << " successfully loaded";
        }
    }

    /*

    // Descriptors - Descriptors layout are needed before creating the pipeline layout

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

    // CREATE BUFFERS

    // Uniform buffer for scene data: two scene params (one for each frame) in same buffer
    const size_t sceneParamBufferSize = FRAME_OVERLAP * padUniformBufferSize(context, sizeof(render::vk::GPUSceneData));
    sceneBuffer.init(context, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, sceneParamBufferSize);
    // Buffer for camera data
    cameraBuffer.init(context, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VMA_MEMORY_USAGE_CPU_TO_GPU, sizeof(render::vk::GPUCameraData));
    // Buffet for objects
    objectBuffer.init(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                      VMA_MEMORY_USAGE_CPU_TO_GPU, sizeof(render::vk::GPUObjectData) * MAX_OBJECTS);

    for (i32 i = 0; i < FRAME_OVERLAP; ++i) {
        // Allocate one descriptor for each frame, global set layout
        VkDescriptorSetAllocateInfo allocInfo {};
        allocInfo.pNext = nullptr;
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorSetCount = 1;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &globalSetLayout;
        VK_CHECK(vkAllocateDescriptorSets(context.device, &allocInfo, &globalDescriptors[i]));

        // Object set layout
        VkDescriptorSetAllocateInfo objectSetAllocInfo {};
        objectSetAllocInfo.pNext = nullptr;
        objectSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        objectSetAllocInfo.descriptorSetCount = 1;
        objectSetAllocInfo.descriptorPool = descriptorPool;
        objectSetAllocInfo.pSetLayouts = &objectSetLayout;
        VK_CHECK(vkAllocateDescriptorSets(context.device, &objectSetAllocInfo, &objectDescriptors[i]));

        // Make the descriptor point into the camera buffer
        VkDescriptorBufferInfo cameraInfo {};
        // Descriptor will point to the camera buffer
        cameraInfo.buffer = cameraBuffer.handle;
        // ...at 0 offset
        cameraInfo.offset = 0;
        // ...of the size of camera data struct
        cameraInfo.range = sizeof(render::vk::GPUCameraData);

        // Make the descriptor point into the scene buffer
        VkDescriptorBufferInfo sceneInfo {};
        sceneInfo.buffer = sceneBuffer.handle;
        sceneInfo.offset = 0; // For non dynamic buffer, would be: padUniformBufferSize(sizeof(render::vk::GPUSceneData)) * i;
        sceneInfo.range = sizeof(render::vk::GPUSceneData);

        // Make the object descriptor point to the object storage buffer
        VkDescriptorBufferInfo objectBufferInfo {};
        objectBufferInfo.buffer = objectBuffer.handle;
        objectBufferInfo.offset = 0;
        objectBufferInfo.range = sizeof(render::vk::GPUObjectData) * MAX_OBJECTS;

    }
    */

    // Pipeline layout
    initPipelineLayout(setLayouts);

    // Pipeline
    initPipeline(context, renderpass);


}

bool Shader::load(const Context& context, const string& shaderName, const string& typeString,
                  VkShaderStageFlagBits shaderStageFlagBit,
                  ShaderStage& shaderStage) {

    string path { "../../shaders/" + shaderName + "." + typeString + ".spv" };

    // Open the file stream in binary mode and put cursor at end
    std::ifstream file { path, std::ios::ate | std::ios::binary };
    if (!file.is_open()) {
        LOG(LogLevel::Error) << "Cannot read shader file " << path;
        return false;
    }
    // Find what the size of the file is by looking up the location of the cursor, which is end of file.
    auto fileSize = file.tellg();
    // SpirV expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file.
    vector<u32> buffer(fileSize / sizeof(u32));
    // Put file cursor at beginning.
    file.seekg(0);
    // Load the entire file into the buffer.
    file.read((char*) buffer.data(), fileSize);
    file.close();

    // Create a new shader module, using the buffer we loaded
    shaderStage.moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderStage.moduleCreateInfo.pNext = nullptr;
    shaderStage.moduleCreateInfo.codeSize = buffer.size() * sizeof(u32);
    shaderStage.moduleCreateInfo.pCode = buffer.data();

    // Check that the creation goes well.
    if (vkCreateShaderModule(context.device, &shaderStage.moduleCreateInfo, nullptr, &shaderStage.moduleHandle) !=
        VK_SUCCESS) {
        return false;
    }

    shaderStage.stageCreateInfo = render::vk::pipelineShaderStageCreateInfo(shaderStageFlagBit,
                                                                            shaderStage.moduleHandle, "main");

    /*
    context.mainDeletionQueue.pushFunction([&]() {
       destroy();
    });
     */
    return true;
}

void Shader::destroy() {
    sceneBuffer.destroy();
    cameraBuffer.destroy();
    objectBuffer.destroy();
    pipeline.destroy();
    for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
        vkDestroyShaderModule(contextDevice, stages[i].moduleHandle, nullptr);
    }
}

vector<VkPipelineShaderStageCreateInfo> Shader::getStagesCreateInfo() {
    vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
        shaderStages.push_back(stages[i].stageCreateInfo);
    }
    return shaderStages;
}

void Shader::use(const CommandBuffer& cmd, VkPipelineBindPoint bindPoint) const {
    pipeline.bind(cmd, bindPoint);
}

void Shader::initPipelineLayout(const array<VkDescriptorSetLayout, 3>& setLayouts) {
    // The pipeline layout that controls the inputs/outputs of the shader
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = render::vk::pipelineLayoutCreateInfo();

    // Setup push constants
    VkPushConstantRange pushConstant;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(render::vk::MeshPushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

    // Descriptor sets
    //const array<VkDescriptorSetLayout, 3>& setLayouts { globalSetLayout, objectSetLayout, singleTextureSetLayout };
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();

    VK_CHECK(vkCreatePipelineLayout(contextDevice, &pipelineLayoutInfo, nullptr, &pipeline.layoutHandle));
}

void engine::render::vk::Shader::initPipeline(const Context& context, const Renderpass& renderpass) {
    PipelineBuilder pipelineBuilder;

    // Clear the shader stages for the builder
    pipelineBuilder.shaderStages.clear();
    pipelineBuilder.shaderStages = getStagesCreateInfo();

    // Vertex input controls how to read vertices from vertex buffers.
    render::vk::VertexInputDescription vertexDescription = Vertex::getVertexDescription();

    // Connect the pipeline builder vertex input info to the one we get from Vertex
    pipelineBuilder.vertexInputInfo = render::vk::vertexInputStateCreateInfo();
    pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
    pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();
    pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
    pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

    // Input assembly is the configuration for drawing triangle lists, strips, or individual points.
    pipelineBuilder.inputAssembly = render::vk::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    // Build viewport and scissor from the swapchain extents
    pipelineBuilder.viewport.x = 0.0f;
    pipelineBuilder.viewport.y = 0.0f;
    pipelineBuilder.viewport.width = (float) context.windowExtent.width;
    pipelineBuilder.viewport.height = (float) context.windowExtent.height;
    pipelineBuilder.viewport.minDepth = 0.0f;
    pipelineBuilder.viewport.maxDepth = 1.0f;

    pipelineBuilder.scissor.offset = { 0, 0 };
    pipelineBuilder.scissor.extent = context.windowExtent;

    // Multisampling
    pipelineBuilder.multisampling = render::vk::multisamplingStateCreateInfo();

    // Blend attachments
    pipelineBuilder.colorBlendAttachment = render::vk::colorBlendAttachmentState();

    // Depth Stencil
    pipelineBuilder.depthStencil = render::vk::depthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    // Build pipeline & material
    pipelineBuilder.pipelineLayout = pipeline.layoutHandle;
    pipeline.init(pipelineBuilder, context, renderpass, false);
}

void Shader::updateGlobalState(CommandBuffer& commandBuffer, Buffer& buffer, u64 dataSize, void* data,
                               VkDescriptorSet descriptorSet, u32 firstSet, u32 descriptorSetCount, VkDescriptorType descriptorType,
                               u32 dynamicOffsetCount, u32 dynamicOffsets) {

    vkCmdBindDescriptorSets(commandBuffer.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layoutHandle,
                            firstSet, descriptorSetCount,
                            &descriptorSet, dynamicOffsetCount, &dynamicOffsets);

    buffer.loadData(data, dataSize);

    VkDescriptorBufferInfo bufferInfo {};
    bufferInfo.buffer = buffer.handle;
    bufferInfo.offset = 0;  // Dynamic buffer object
    bufferInfo.range = dataSize;

    VkWriteDescriptorSet write = render::vk::writeDescriptorBuffer(descriptorType, descriptorSet,
                                                                         &bufferInfo, firstSet);

    vkUpdateDescriptorSets(contextDevice, 1, &write, 0, nullptr);
}

