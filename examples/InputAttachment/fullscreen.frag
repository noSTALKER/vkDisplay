#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 texCoord;
layout (location = 0) out vec4 outColor;

layout (input_attachment_index = 0, set = 0, location = 0) uniform subpassInput inputAttachment;

void main()
{
    //invert the color
	vec4 texColor = subpassLoad(inputAttachment);
	outColor = vec4(1,1,1,1) - vec4(texColor.rgb, 0);
}