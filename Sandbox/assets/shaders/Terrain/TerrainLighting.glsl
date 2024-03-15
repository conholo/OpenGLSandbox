#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_ModelMatrix;
uniform mat4 u_MVP;

out vec2 v_UV;
out vec3 v_Normal;
out vec3 v_WorldSpacePosition;
out float v_LocalSpaceHeight;

void main()
{
	v_LocalSpaceHeight = a_Position.y;
	v_WorldSpacePosition = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
	v_Normal = vec3(u_ModelMatrix * vec4(a_Normal, 0.0));
	v_UV = a_TexCoord;
	gl_Position = u_MVP * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

const float Epsilon = 1e-4;

struct Light
{
	vec3 WorldSpacePosition;
	vec3 Color;
	float Intensity;
};

in vec2 v_UV;
in vec3 v_Normal;
in vec3 v_WorldSpacePosition;
in float v_LocalSpaceHeight;

uniform sampler2D u_Texture;
uniform vec3 u_Color;
uniform Light u_Light;
uniform vec3 u_CameraPosition;

uniform int u_LayerCount;

uniform float u_MaxHeight;
uniform float u_MinHeight;

uniform sampler2D u_HeightTextures[7];
uniform float u_Blends[7];
uniform float u_HeightThresholds[7];
uniform float u_TextureTiling[7];
uniform vec3 u_TintColors[7];

float InverseLerp(float a, float b, float v)
{
	return max(0.0, (v - a) / (b - a));
}

void main()
{
	vec3 normal = normalize(v_Normal);
	vec3 directionToLight = normalize(u_Light.WorldSpacePosition - v_WorldSpacePosition);

	float diffuse = max(0.0, dot(normal, directionToLight));

	vec3 textureBlend = vec3(1.0);
	vec3 blendNormal = normal / normal.x + normal.y + normal.z;

	float heightPercent = InverseLerp(u_MinHeight, u_MaxHeight, v_LocalSpaceHeight);

	for (int i = 0; i < u_LayerCount; i++)
	{
		float strength = InverseLerp(-u_Blends[i] / 2.0 - Epsilon, u_Blends[i] / 2.0, heightPercent - u_HeightThresholds[i]);
		textureBlend = textureBlend * (1.0 - strength) + (texture(u_HeightTextures[i], v_UV * u_TextureTiling[i]).rgb * u_TintColors[i]) * strength;
	}

	vec3 lighting = u_Light.Color * u_Light.Intensity * diffuse * textureBlend;

	o_Color = vec4(u_Color * lighting, 1.0);
}