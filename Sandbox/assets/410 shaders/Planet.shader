#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_MVP;
uniform mat4 u_NormalMatrix;
uniform mat4 u_Transform;

out vec3 v_Normal;
out vec3 v_WorldSpacePosition;

void main()
{
	v_Normal = (u_NormalMatrix * vec4(a_Normal, 0.0)).xyz;
	v_WorldSpacePosition = (u_Transform * vec4(a_Position, 1.0)).xyz;
	gl_Position = u_MVP * vec4(a_Position, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

struct Light
{
	vec3 Position;
	vec3 Color;
	float Intensity;
};

layout(std140, binding = 0) uniform PlanetMaterialProperties
{
	vec3 AmbientColor;
	float AmbientStrength;

	vec3 DiffuseColor;
	float DiffuseStrength;

	float SpecularStrength;
	float Shininess;
};

uniform Light u_Light;
uniform vec3 u_Color;
uniform vec3 u_CameraPosition;

in vec3 v_Normal;
in vec3 v_WorldSpacePosition;

void main()
{
	vec3 normal = normalize(v_Normal);
	vec3 lightDirection = normalize(u_Light.Position - v_WorldSpacePosition);
	vec3 viewDirection = normalize(u_CameraPosition - v_WorldSpacePosition);

	float ndotl = max(0.0, dot(normal, lightDirection));

	vec3 diffuse = DiffuseColor * ndotl * DiffuseStrength;

	vec3 halfway = normalize(viewDirection + lightDirection);
	float specularValue = pow(max(0.0, dot(normal, halfway)), Shininess * Shininess);
	vec3 specular = u_Light.Color * specularValue * SpecularStrength;

	vec3 ambient = AmbientColor * AmbientStrength;

	vec3 lighting = (diffuse + specular + ambient) * u_Light.Color * u_Light.Intensity;

	o_Color = vec4(lighting, 1.0);
}
