#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

layout (std140, binding = 0) uniform ModelInfo {
	mat4 mvp;
} modelInfo;

out gl_PerVertex {
	vec4 gl_Position;
};

layout (location = 0) out vec4 outNormal;

void main()
{
	gl_Position = modelInfo.mvp * vec4(position, 1.0f);
	outNormal = modelInfo.mvp * vec4(normal, 0.0f);
}