#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

out vec3 v_TexCoords;

uniform mat4 u_ViewProjection;

void main()
{
	v_TexCoords = a_Position;
	vec4 position = u_ViewProjection * vec4(a_Position, 1.0f);
	gl_Position = position.xyww;
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec3 v_TexCoords;

uniform samplerCube u_Skybox;

void main()
{
	o_Color = texture(u_Skybox, v_TexCoords);
}