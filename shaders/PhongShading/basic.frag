#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

struct LightInfo {
	vec4 direction;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct MaterialInfo {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float specularPower;
};

layout (push_constant) uniform Infos {
	LightInfo lightInfo;
	MaterialInfo matInfo;
} infos;

layout (location = 0) in vec4 viewNormal;
layout (location = 1) in vec4 viewPosition;

layout (location = 0) out vec4 outColor;

void main()
{
	vec4 normal = normalize(viewNormal);
	vec4 reflectedRay = normalize(reflect(infos.lightInfo.direction, viewNormal));
	outColor = infos.lightInfo.ambient * infos.matInfo.ambient;
	outColor += infos.lightInfo.diffuse * infos.matInfo.diffuse * clamp(dot(viewNormal, reflectedRay), 0, 1);

	float v_dot_r = clamp(dot(reflectedRay, normalize(-viewPosition)), 0, 1);
	outColor += infos.lightInfo.specular * infos.matInfo.specular * pow(v_dot_r, infos.matInfo.specularPower);
	outColor.w = 1;
}