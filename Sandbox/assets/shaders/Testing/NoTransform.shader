#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec2 a_Normal;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

void main()
{
	o_Color = vec4(1.0);
}