#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

layout (std140, binding = 0) uniform CubeInfo {
	mat4 mvp;
} cubeInfo;

layout (location = 0) out vec2 outTexCoord;
out gl_PerVertex {
	vec4 gl_Position;
};

void main()
{
	vec4 pos = vec4(position, 1.0f);
	gl_Position = cubeInfo.mvp * pos;
	outTexCoord = texCoord;
}