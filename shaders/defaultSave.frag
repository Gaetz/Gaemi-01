#version 450

// Color input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;

// Texture sampler
layout (set = 2, binding = 0) uniform sampler2D tex1;

layout(set = 0, binding = 1) uniform SceneData {   
    vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
} sceneData;

// Output color
layout (location = 0) out vec4 outColor;


void main()
{
    //outColor = vec4(inColor + sceneData.ambientColor.xyz, 1.0f);
	vec3 color = texture(tex1,texCoord).xyz;
	outColor = vec4(color,1.0f);
}