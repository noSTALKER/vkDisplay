#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

layout (push_constant) uniform CubeInfo {
	mat4 mvp[3];
} cubeInfo;

layout (location = 0) out vec2 outTexCoord;
out gl_PerVertex {
	vec4 gl_Position;
};

void main()
{
	vec4 pos = vec4(position, 1.0f);
	gl_Position = cubeInfo.mvp[gl_InstanceIndex] * pos;
	outTexCoord = texCoord;
}