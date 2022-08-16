#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

layout(std140, binding = 1) uniform Camera
{
	mat4 u_ViewProjectionMatrix;
	mat4 u_ModelMatrix;
	mat4 u_NormalMatrix;
};

layout(std140, binding = 2) uniform Shadow
{
	mat4 u_ShadowMatrix;
};

void main()
{
	gl_Position = u_ShadowMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
}


#type fragment
#version 450

void main()
{
}