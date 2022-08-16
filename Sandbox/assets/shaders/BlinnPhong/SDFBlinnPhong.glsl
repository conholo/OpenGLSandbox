#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;

out vec3 v_ModelPosition;
out vec2 v_TexCoord;
out vec3 v_RayOrigin;
out mat4 v_ModelMatrix;

void main()
{
	v_ModelPosition = a_Position;
	v_TexCoord = a_TexCoord;

	gl_Position = u_MVP * vec4(a_Position, 1.0);
}

#type fragment

#version 450 core

layout(location = 0) out vec4 o_Color;

struct MaterialProperties
{
	vec3 DiffuseColor;
	vec3 AmbientColor;
	float AmbientStrength;
	float DiffuseStrength;
	float SpecularStrength;
	float Shininess;
};

struct DirectionalLight
{
	vec3 LightPosition;
	vec3 LightColor;
	float Intensity;
};

struct SpotLight
{
	vec3 LightPosition;
	vec3 LightColor;
	vec3 LightDirection;
	float Intensity;
	float ConstantAttenuation;
	float LinearAttenuation;
	float QuadraticAttenuation;
	float InnerCutOff;
	float OuterCutOff;
};

struct PointLight
{
	vec3 LightPosition;
	vec3 LightColor;
	float Intensity;
	float ConstantAttenuation;
	float LinearAttenuation;
	float QuadraticAttenuation;
};

#define POINT_LIGHT_COUNT 15
#define SPOT_LIGHT_COUNT 10


uniform MaterialProperties u_MaterialProperties;
uniform DirectionalLight u_DirectionalLight;
uniform SpotLight u_SpotLights[SPOT_LIGHT_COUNT];
uniform PointLight u_PointLights[POINT_LIGHT_COUNT];
uniform sampler2D u_Texture;

#define STEP_COUNT 100
#define MAX_DISTANCE 100
#define MIN_DISTANCE 0.01

uniform float u_SmoothUnionCoefficient;
uniform vec3 u_CameraPosition;
uniform vec3 u_CameraPositionWorld;
uniform vec3 u_SphereOffset;

uniform int u_UseDirectionalLight;
uniform int u_UsePointLights;
uniform int u_UseSpotLights;

in mat4 v_ModelMatrix;
in vec3 v_ModelPosition;
in vec2 v_TexCoord;


vec3 CalculateBlinnPhong(vec3 normal, MaterialProperties properties, vec3 lightColor, vec3 lightPosition, float lightIntensity)
{
	vec3 lightDirection = normalize(lightPosition - v_ModelPosition);

	// Ambient
	vec3 ambient = lightIntensity * mix(lightColor, properties.AmbientColor, properties.AmbientStrength) * properties.AmbientStrength;

	// Diffuse
	float NdotL = max(dot(normal, lightDirection), 0);
	vec3 diffuse = lightIntensity * NdotL * properties.DiffuseStrength * lightColor;

	// Specular
	vec3 inverseView = normalize(u_CameraPosition - v_ModelPosition);
	vec3 halfway = normalize(lightDirection + inverseView);
	float specular = pow(max(dot(normal, halfway), 0.0), properties.Shininess);
	vec3 spec = lightIntensity * specular * lightColor * properties.SpecularStrength;

	return (ambient + diffuse + spec) * properties.DiffuseColor;
}


vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal)
{
	return CalculateBlinnPhong(normal, u_MaterialProperties, light.LightColor, light.LightPosition, light.Intensity);
}


vec3 CalculatePointLight(PointLight light, vec3 normal)
{
	vec3 result = CalculateBlinnPhong(normal, u_MaterialProperties, light.LightColor, light.LightPosition, light.Intensity);

	float distance = length(light.LightPosition - v_ModelPosition);
	float attenuation = 1 / (light.ConstantAttenuation + distance * light.LinearAttenuation + distance * distance * light.QuadraticAttenuation);
	return result * attenuation;
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal)
{
	vec3 result = CalculateBlinnPhong(normal, u_MaterialProperties, light.LightColor, light.LightPosition, light.Intensity);

	vec3 directionToLight = normalize(light.LightPosition - v_ModelPosition);
	float theta = dot(directionToLight, normalize(-light.LightDirection));
	float epsilon = light.InnerCutOff - light.OuterCutOff;
	float intensity = smoothstep(0.0, 1.0, (theta - light.OuterCutOff) / epsilon);

	float distance = length(light.LightPosition - v_ModelPosition);
	float attenuation = 1 / (light.ConstantAttenuation + distance * light.LinearAttenuation + distance * distance * light.QuadraticAttenuation);

	return result * attenuation * intensity;
}


//https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
float OpSmoothUnion(float d1, float d2, float k)
{
	float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
	return mix(d2, d1, h) - k * h * (1.0 - h);
}


float SceneDistance(vec3 position)
{
	float sphereDistance = length(position + u_SphereOffset) - 0.075;
	float sphere2Distance = length(position + u_SphereOffset.yxz * 1.2f) - 0.04;
	float torusDistance = length(vec2(length(position.xz) - 0.2, position.y)) - .015;

	return OpSmoothUnion(OpSmoothUnion(sphereDistance, torusDistance, u_SmoothUnionCoefficient), sphere2Distance, u_SmoothUnionCoefficient / 2.0);
}

vec3 SceneNormals(vec3 position)
{
	vec2 epsilon = vec2(0.00001, 0.0);

	vec3 normal = SceneDistance(position) - vec3(
		SceneDistance(position - epsilon.xyy),
		SceneDistance(position - epsilon.yxy),
		SceneDistance(position - epsilon.yyx)
	);

	return normalize(normal);
}

float RayMarch(vec3 rayOrigin, vec3 rayDirection)
{
	float distanceFromOrigin = 0.0;

	for (int i = 0; i < STEP_COUNT; i++)
	{
		vec3 position = rayOrigin + rayDirection * distanceFromOrigin;

		float distanceFromScene = SceneDistance(position);

		distanceFromOrigin += distanceFromScene;

		if (distanceFromScene < MIN_DISTANCE || distanceFromOrigin > MAX_DISTANCE) break;
	}

	return distanceFromOrigin;
}

void main()
{
	vec3 rayOrigin = u_CameraPosition;
	vec3 rayDirection = normalize(v_ModelPosition - rayOrigin);

	float rayMarch = RayMarch(rayOrigin, rayDirection);

	vec3 result = vec3(0.0);

	if (rayMarch >= MAX_DISTANCE)
		discard;
	else
	{
		vec3 rayMarchPosition = rayOrigin + rayDirection * rayMarch;

		vec3 normal = SceneNormals(rayMarchPosition);

		result = u_UseDirectionalLight == 1 ? CalculateDirectionalLight(u_DirectionalLight, normal) : vec3(0.0);

		if (u_UsePointLights == 1)
		{
			for (int i = 0; i < POINT_LIGHT_COUNT; i++)
				result += CalculatePointLight(u_PointLights[i], normal);
		}

		if (u_UseSpotLights == 1)
		{
			for (int i = 0; i < SPOT_LIGHT_COUNT; i++)
				result += CalculateSpotLight(u_SpotLights[i], normal);
		}

		float fogMaxDistance = 100.0f;
		float fogMinDistance = 0.1f;

		vec3 worldPosition = (v_ModelMatrix * vec4(v_ModelPosition, 1.0)).xyz;

		float distance = length(worldPosition - u_CameraPositionWorld);

		vec3 fogColor = vec3(0.1, 0.1, 0.5);
		float fogFactor = 1 - clamp((fogMaxDistance - distance) / (fogMaxDistance - fogMinDistance), 0.0, 1.0);

		result = mix(result, fogColor, fogFactor);
	}

	o_Color = vec4(result, 1.0);
}