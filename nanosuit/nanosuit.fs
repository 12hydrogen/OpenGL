#version 330 core

in vec3 aNormal;
in vec3 fragPos;
in vec2 aTexture;

out vec4 FragColor;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct light {
	vec3 pos;
	vec3 color;
	vec3 direction;

	vec3 strength;
	vec3 attenuation;

	float cutoff;
	float outerCutoff;
};

uniform Material material;
uniform light white;
uniform light white_parallel;
uniform light white_spotlight;

uniform vec3 viewPos;
uniform vec3 viewFacing;

uniform sampler2D diffuseTexture_0;
uniform sampler2D specularTexture_0;

// 0 == Dot
// 1 == Parallel
// 2 == Spot
vec3 phongModel(light lighting, vec4 diffTexture, vec4 specTexture, int type)
{
	lighting.direction = normalize(lighting.direction);

	float dist = length(lighting.pos - fragPos);
	float attenuation = 1.0 / (lighting.attenuation[0] + lighting.attenuation[1] * dist + lighting.attenuation[2] * (dist * dist));

	vec3 ambient = lighting.color * lighting.strength[0] * vec3(diffTexture);

	vec3 lightRelative;
	if (type == 1)
		lightRelative = normalize(-lighting.direction);
	else
		lightRelative = normalize(lighting.pos - fragPos);

	float theta = dot(lightRelative, normalize(-lighting.direction));
	float epsilon = lighting.cutoff - lighting.outerCutoff;
	float intensity = clamp((theta - lighting.outerCutoff) / epsilon, 0.0, 1.0);

	if ((theta > lighting.outerCutoff && type == 2) || type != 2)
	{
		vec3 normal = normalize(aNormal);
		float diffusion = max(dot(normal, lightRelative), 0.0);
		vec3 diffuse = lighting.color * lighting.strength[1] * diffusion * vec3(diffTexture);

		vec3 viewDir = normalize(viewPos - fragPos);
		vec3 reflectRelative = reflect(-lightRelative, normal);
		float specularation = pow(max(dot(viewDir, reflectRelative), 0.0), material.shininess);
		vec3 specular = lighting.color * lighting.strength[2] * specularation * vec3(specTexture);

		if (type == 0)
			ambient *= attenuation;
		if (type != 1)
		{
			diffuse *= attenuation;
			specular *= attenuation;
		}
		if (type == 2)
		{
			diffuse *= intensity;
			specular *= intensity;
		}

		return ambient + diffuse + specular;
	}
	else
	{
		return ambient;
	}
}

vec3 flashlight(float cutoff, float outerCutoff, vec3 color, vec3 attenuation, vec3 strength, vec4 diffTexture, vec4 specTexture)
{
	light fl;
	fl.pos = viewPos;
	fl.direction = viewFacing;
	fl.cutoff = cos(radians(cutoff));
	fl.outerCutoff = cos(radians(outerCutoff));
	fl.color = color;
	fl.strength = strength;
	fl.attenuation = attenuation;
	return phongModel(fl, diffTexture, specTexture, 2);
}

void main()
{
	vec4 diffTex = texture(diffuseTexture_0, aTexture);
	vec4 specTex = texture(specularTexture_0, aTexture);
	// vec4 diffTex = vec4(1.0, 1.0, 1.0, 1.0);
	// vec4 specTex = vec4(1.0, 1.0, 1.0, 1.0);
	vec3 result = vec3(0.0, 0.0, 0.0);
	result += phongModel(white, diffTex, specTex, 0);
	// result += phongModel(white_parallel, diffTex, specTex, 1);
	result += phongModel(white_spotlight, diffTex, specTex, 2);
	result += flashlight(5.0, 10.0, vec3(1.0), white_spotlight.attenuation, white_spotlight.strength, diffTex, specTex);
	FragColor = vec4(result, 1.0f);
}
