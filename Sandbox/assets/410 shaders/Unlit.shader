#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

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

uniform vec3 u_Color;
uniform float u_DepthSlice;
uniform sampler3D u_Texture;

in vec2 v_TexCoord;

void main()
{
	o_Color = texture(u_Texture, vec3(v_TexCoord, u_DepthSlice)) * vec4(u_Color, 1.0);
}
