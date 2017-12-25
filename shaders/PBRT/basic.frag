#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

const float PI = 3.14159265359;

struct LightInfo {
	vec4 direction;
	vec4 color;
};

struct MaterialInfo {
	vec4 albedo;
	float metallic;
	float roughness;
};

layout (push_constant) uniform Infos {
	LightInfo lightInfo;
	MaterialInfo matInfo;
} infos;

layout (location = 0) in vec4 viewNormal;
layout (location = 1) in vec4 viewPosition;

layout (location = 0) out vec4 outColor;

float distributionGGX(float ndoth, float roughness)
{
	float roughness2 = roughness * roughness;
	float roughness4 = roughness2 * roughness2;
	float ndoth2 = ndoth * ndoth;
	float denominator = (ndoth2 * (roughness4 - 1.0) + 1.0);
	denominator = PI * denominator * denominator;
	return roughness4 / denominator;
}

float geometrySchlickGGX(float cosTheta, float roughness)
{
	float r = (roughness + 1);
	float k = (r * r) / 8.0;
	float denominator = cosTheta * (1.0 - k) + k;
	return cosTheta / denominator;
}

float geometrySmith(float ndotv, float ndotl, float roughness)
{
	float ggx1 = geometrySchlickGGX(ndotv, roughness);
	float ggx2 = geometrySchlickGGX(ndotl, roughness);
	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
	vec3 normal = normalize(viewNormal.xyz);
	vec3 viewDir = normalize(-viewPosition.xyz);
	vec3 f0 = vec3(0.04);
	f0 = mix(f0, infos.matInfo.albedo.rgb, infos.matInfo.metallic);
	vec3 lo = vec3(0.0);

	vec3 lightDir = normalize(-infos.lightInfo.direction.xyz);
	vec3 halfDir = normalize(lightDir + viewDir);
	float hdotv = max(dot(halfDir, viewDir), 0.0);
	float ndotl = max(dot(normal, lightDir), 0.0);
	float ndotv = max(dot(normal, viewDir), 0.0);
	float ndoth = max(dot(normal, halfDir), 0.0);

	float ndf = distributionGGX(ndoth, infos.matInfo.roughness);
	float g = geometrySmith(ndotv, ndotl, infos.matInfo.roughness);
	vec3 f = fresnelSchlick(hdotv, f0);

	vec3 kS = f;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - infos.matInfo.metallic;

	vec3 nominator = ndf * g * f;
	float denominator = 4 * ndotv * ndotl + 0.00001;
	vec3 specular = nominator / denominator;

	lo += (kD * infos.matInfo.albedo.rgb / PI + specular) * infos.lightInfo.color.rgb * ndotl;
	vec3 ambient = vec3(0.03) * infos.matInfo.albedo.rgb;
	vec3 color = lo + ambient;

	color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  

	outColor = vec4(color, 1);
}