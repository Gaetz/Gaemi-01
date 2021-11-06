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

void Shader::init(const Context& context,
                  const array<VkDescriptorSetLayout, 3>& setLayouts,
                  const Renderpass& renderpass,
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

    // Pipeline layout
    initPipelineLayout(context, setLayouts);

    // Pipeline
    initPipeline(context, renderpass);

    // Descriptors

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
    for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
        vkDestroyShaderModule(contextDevice, stages[i].moduleHandle, nullptr);
    }
    pipeline.destroy();
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

void Shader::initPipelineLayout(const Context& context, const array<VkDescriptorSetLayout, 3>& setLayouts) {
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
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();

    VK_CHECK(vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr, &pipeline.layoutHandle));
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

