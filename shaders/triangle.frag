#version 450

// Color input
layout (location = 0) in vec3 inColor;

// Output color
layout (location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(inColor, 1.0f);
}