#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_NormalMatrix;


out vec3 v_ViewPosition;
out vec3 v_NormalSmooth;
flat out vec3 v_NormalFlat;
out vec2 v_TexCoord;

void main()
{
	vec4 viewPosition = u_ViewMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
	v_ViewPosition = viewPosition.xyz;
	v_NormalSmooth = v_NormalFlat = vec3(u_NormalMatrix * vec4(a_Normal, 0.0));
	v_TexCoord = a_TexCoord;

	gl_Position = u_ProjectionMatrix * u_ViewMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
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

uniform int u_UseDirectionalLight;
uniform int u_UsePointLights;
uniform int u_UseSpotLights;

uniform MaterialProperties u_MaterialProperties;
uniform DirectionalLight u_DirectionalLight;
uniform SpotLight u_SpotLights[SPOT_LIGHT_COUNT];
uniform PointLight u_PointLights[POINT_LIGHT_COUNT];
uniform sampler2D u_Texture;
uniform int u_IsSmooth = 1;

in vec3 v_ViewPosition;
in vec3 v_NormalSmooth;
flat in vec3 v_NormalFlat;
in vec2 v_TexCoord;


vec3 CalculateBlinnPhong(vec3 normal, MaterialProperties properties, vec3 lightColor, vec3 lightViewSpacePosition, float lightIntensity)
{
	vec3 lightDirection = normalize(lightViewSpacePosition - v_ViewPosition);

	// Ambient
	vec3 ambient = lightIntensity * mix(lightColor, properties.AmbientColor, properties.AmbientStrength) * properties.AmbientStrength;

	// Diffuse
	float NdotL = max(dot(normal, lightDirection), 0);
	vec3 diffuse = lightIntensity * NdotL * properties.DiffuseStrength * lightColor;

	// Specular
	vec3 inverseView = normalize(-v_ViewPosition);
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
	
	float distance = length(light.LightPosition - v_ViewPosition);
	float attenuation = 1 / (light.ConstantAttenuation + distance * light.LinearAttenuation + distance * distance * light.QuadraticAttenuation);
	return result * attenuation;
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal)
{
	vec3 result = CalculateBlinnPhong(normal, u_MaterialProperties, light.LightColor, light.LightPosition, light.Intensity);
	vec3 directionToLight = normalize(light.LightPosition - v_ViewPosition);
	float theta = dot(directionToLight, normalize(-light.LightDirection));
	float epsilon = light.InnerCutOff - light.OuterCutOff;
	float intensity = smoothstep(0.0, 1.0, (theta - light.OuterCutOff) / epsilon);

	float distance = length(light.LightPosition - v_ViewPosition);
	float attenuation = 1 / (light.ConstantAttenuation + distance * light.LinearAttenuation + distance * distance * light.QuadraticAttenuation);

	return result * attenuation * intensity;
}

void main()
{
	vec3 normals = u_IsSmooth == 1 ? v_NormalSmooth : v_NormalFlat;
	vec3 normal = normalize(normals);

	vec3 result = u_UseDirectionalLight == 1 ? CalculateDirectionalLight(u_DirectionalLight, normal) : vec3(0.0);

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

	vec3 albedo = texture(u_Texture, v_TexCoord).xyz;

	albedo *= result;

	float fogMaxDistance = 100.0f;
	float fogMinDistance = 0.1f;

	float distance = length(v_ViewPosition);

	vec3 fogColor = vec3(0.1, 0.1, 0.5);
	float fogFactor = 1 - clamp((fogMaxDistance - distance) / (fogMaxDistance - fogMinDistance), 0.0, 1.0);

	result = mix(albedo, fogColor, fogFactor);

	o_Color = vec4(result, 1.0);
}