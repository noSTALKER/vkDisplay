#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

layout (location = 0) out vec2 outTexCoord;
out gl_PerVertex {
	vec4 gl_Position;
};

void main()
{
	gl_Position = vec4(position, 0.5, 1);
	outTexCoord = texCoord;
}