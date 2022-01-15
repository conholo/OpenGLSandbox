#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_ModelMatrix;
uniform mat4 u_NormalMatrix;
uniform mat4 u_MVP;

out vec2 v_UV;
out vec3 v_Normal;
out vec3 v_WorldSpacePosition;

void main()
{
	v_WorldSpacePosition = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
	v_Normal = vec3(u_ModelMatrix * vec4(a_Normal, 0.0));
	v_UV = a_TexCoord;
	gl_Position = u_MVP * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct Light
{
	vec3 WorldSpacePosition;
	vec3 Color;
	float Intensity;
};

in vec2 v_UV;
in vec3 v_Normal;
in vec3 v_WorldSpacePosition;

uniform sampler2D u_Texture;
uniform vec3 u_Color;
uniform Light u_Light;
uniform vec3 u_CameraPosition;

void main()
{
	vec3 normal = normalize(v_Normal);
	vec3 directionToLight = normalize(u_Light.WorldSpacePosition - v_WorldSpacePosition);

	float diffuse = max(0.0, dot(normal, directionToLight));

	vec3 lighting = u_Light.Color * u_Light.Intensity * diffuse;

	o_Color = vec4(texture(u_Texture, v_UV).rgb * u_Color * lighting, 1.0);
}