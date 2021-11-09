#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec2 a_Normal;

uniform mat4 u_MVP;
out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = u_MVP * vec4(a_Position, 1.0);
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

uniform float u_Scale;
uniform float u_Size;

float Grid(vec2 st, float resolution)
{
	vec2 grid = fract(st);
	return step(resolution, grid.x) * step(resolution, grid.y);
}


void main()
{
	float x = Grid(v_TexCoord * u_Scale, u_Size);

	float dist = length(v_TexCoord * 2.0 - 1.0);

	float fade = mix(0, 1, dist);
	o_Color = vec4(vec3(0.2), fade) * (1.0 - x);

	if (o_Color.a == 0.0)
		discard;
}