#version 460

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 texCoord;

layout (set = 0, binding = 0) uniform CameraBuffer
{
    mat4 view;
    mat4 projection;
    mat4 viewProj;
} cameraData;

struct ObjectData {
    mat4 model;
};

// All object matrices
layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
    ObjectData objects[];
} objectBuffer;

// Push constants, Unused
layout (push_constant) uniform constants
{
    vec4 data;
    mat4 renderMatrix;
} PushConstants;

void main()
{
    mat4 modelMatrix = objectBuffer.objects[gl_BaseInstance].model;
    mat4 transformMatrix = (cameraData.viewProj * modelMatrix);
    gl_Position = transformMatrix * vec4(vPosition, 1.0f);
    outColor = vColor;
    texCoord = vTexCoord;
}
