//
// Created by gaetz on 26/10/2021.
//

#include "Shader.h"
#include "Context.h"
#include "Init.h"
#include <vector>

using engine::render::vk::Shader;
using std::vector;

Shader::Shader(Context &contextP) : context { contextP } {}

void Shader::init(const string &shaderName) {
    // Shader module and stages
    array<string, SHADER_STAGE_COUNT> typeStrings { "vert", "frag" };
    array<VkShaderStageFlagBits, SHADER_STAGE_COUNT> types { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };

    for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
        if (!load(shaderName, typeStrings[i], types[i], stages[i])) {
            LOG(LogLevel::Error) << "Error when building shader " << shaderName;
        } else {
            LOG(LogLevel::Info) << "Shader " << shaderName << " successfully loaded";
        }
    }

    // Descriptors

}

bool Shader::load(const string &shaderName, const string &typeString,
                  VkShaderStageFlagBits shaderStageFlagBit,
                  ShaderStage &shaderStage) {

    string path { "../../shaders/" + shaderName + "." + typeString + ".spv" };

    // Open the file stream in binary mode and put cursor at end
    std::ifstream file {path, std::ios::ate | std::ios::binary};
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
    if (vkCreateShaderModule(context.device, &shaderStage.moduleCreateInfo, nullptr, &shaderStage.moduleHandle) != VK_SUCCESS) {
        return false;
    }

    shaderStage.stageCreateInfo = render::vk::pipelineShaderStageCreateInfo(shaderStageFlagBit, shaderStage.moduleHandle, "main");

    return true;
}

void Shader::destroy() {
    for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
        vkDestroyShaderModule(context.device, stages[i].moduleHandle, nullptr);
    }
}

vector<VkPipelineShaderStageCreateInfo> Shader::getStagesCreateInfo() {
    vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (u32 i = 0; i < SHADER_STAGE_COUNT; ++i) {
        shaderStages.push_back(stages[i].stageCreateInfo);
    }
    return shaderStages;
}

void Shader::use() {

}

