#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_MVP;

void main()
{
	gl_Position = u_MVP * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

void main()
{
	o_Color = vec4(1.0, 0.0, 0.0, 1.0);
}