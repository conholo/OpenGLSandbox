#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Normal;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position.xy, 0.0f, 1.0f);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;
uniform sampler2D u_SceneTexture;

void main()
{
	vec3 color = texture(u_SceneTexture, v_TexCoord).rgb;
	o_Color = vec4(color, 1.0);
}