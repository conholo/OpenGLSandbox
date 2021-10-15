#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 0) in vec2 a_TexCoord;
layout(location = 0) in vec3 a_Normal;


out vec3 v_Normal;
out vec3 v_WorldSpacePosition;

uniform mat3 u_NormalMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ModelMatrix;
uniform mat4 u_MVP;

void main()
{
	v_Normal = u_NormalMatrix * a_Normal;
	v_WorldSpacePosition = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
	gl_Position = u_MVP * vec4(a_Position, 1.0f);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec3 v_WorldSpacePosition;
in vec3 v_Normal;

uniform vec3 u_CameraPosition;
uniform samplerCube u_Skybox;

void main()
{
	vec3 incident = normalize(v_WorldSpacePosition - u_CameraPosition);
	vec3 reflection = reflect(incident, normalize(v_Normal));
	o_Color = vec4(texture(u_Skybox, reflection).rgb, 1.0);
}