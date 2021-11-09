#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_MVP;
uniform mat4 u_ModelViewMatrix;
uniform mat4 u_NormalMatrix;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_ViewSpacePosition;

void main()
{
	v_Normal = (u_NormalMatrix * vec4(a_Normal, 1.0)).xyz;
	v_ViewSpacePosition = (u_ModelViewMatrix * vec4(a_Position, 1.0)).xyz;
	v_TexCoord = a_TexCoord;
	gl_Position = u_MVP * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct PointLight
{
	vec3 ViewSpaceLightPosition;
	vec3 AmbientColor;
	vec3 SpecularColor;
	float Intensity;
	float ConstantAttenuation;
	float LinearAttenuation;
	float QuadraticAttenuation;
};

struct MaterialProperties
{
	sampler2D DiffuseTexture;
	sampler2D NormalMap;
	float Shininess;
	float DiffuseStrength;
	float SpecularStrength;
	float AmbientStrength;
};

in vec2 v_TexCoord;
in vec3 v_Normal;

in vec3 v_ViewSpacePosition;

uniform MaterialProperties u_Properties;
uniform PointLight u_PointLight;

vec3 BlinnPhong(PointLight light, MaterialProperties properties, vec3 normal)
{
	vec3 viewDirection = normalize(-v_ViewSpacePosition);
	vec3 directionToLight = normalize(light.ViewSpaceLightPosition - v_ViewSpacePosition);

	vec3 samplerColor = texture(properties.DiffuseTexture, v_TexCoord).rgb;

	float ndotl = max(dot(normal, directionToLight), 0);
	vec3 diffuse = ndotl * samplerColor * properties.DiffuseStrength;

	vec3 ambient = light.AmbientColor * properties.AmbientStrength;

	vec3 inverseView = normalize(-v_ViewSpacePosition);
	vec3 halfway = normalize(directionToLight + inverseView);
	float spec = pow(max(dot(normal, halfway), 0), properties.Shininess);
	vec3 specular = light.SpecularColor * spec * properties.SpecularStrength;

	float distance = length(light.ViewSpaceLightPosition - v_ViewSpacePosition);
	float attenuation = 1 / (light.ConstantAttenuation + distance * light.LinearAttenuation + distance * distance * light.QuadraticAttenuation);

	vec3 result = (ambient + specular + diffuse);

	return result * attenuation * light.Intensity;
}

void main()
{
	vec3 normal = texture(u_Properties.NormalMap, v_TexCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 color = BlinnPhong(u_PointLight, u_Properties, normal);

	o_Color = vec4(color, 1.0);
}
