#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_ModelMatrix;
uniform mat4 u_MVP;

out vec3 v_WorldPosition;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main()
{
	v_WorldPosition = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
	v_Normal = vec3(u_ModelMatrix * vec4(a_Normal, 0.0));
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

struct Light
{
	vec3 Position;
	vec3 Color;
	float Intensity;
};

uniform MaterialProperties u_MaterialProperties;
uniform Light u_Light;
uniform sampler2D u_Texture;
uniform vec3 u_CameraPosition;

in vec3 v_WorldPosition;
in vec3 v_Normal;
in vec2 v_TexCoord;


vec3 BlinnPhong(vec3 normal, Light light, MaterialProperties properties)
{
	vec3 lightDirection = normalize(light.Position - v_WorldPosition);
	vec3 viewDirection = normalize(u_CameraPosition - v_WorldPosition);

	// Ambient
	vec3 ambient = light.Intensity * mix(light.Color, properties.AmbientColor, properties.AmbientStrength);

	// Diffuse
	float NdotL = max(dot(normal, lightDirection), 0);
	vec3 diffuse = light.Intensity * NdotL * properties.DiffuseStrength * light.Color;

	// Specular
	vec3 halfway = normalize(lightDirection + viewDirection);
	float specular = pow(max(dot(normal, halfway), 0.0), properties.Shininess * properties.Shininess);
	vec3 spec = light.Intensity * specular * light.Color * properties.SpecularStrength;

	return (ambient + diffuse + spec) * properties.DiffuseColor;
}


void main()
{
	vec3 normal = normalize(v_Normal);

	vec3 result = BlinnPhong(normal, u_Light, u_MaterialProperties);
	vec3 albedo = texture(u_Texture, v_TexCoord).xyz;

	albedo *= result;

	o_Color = vec4(albedo, 1.0);
}