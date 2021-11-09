#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

out vec3 v_Position;

uniform mat4 u_ViewProjection;

void main()
{
	vec4 position = vec4(a_Position.xy, 1.0, 1.0);
	gl_Position = position;

	v_Position = (u_ViewProjection * position).xyz;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

uniform samplerCube u_Texture;

uniform float u_TextureLOD;
uniform float u_Intensity;

in vec3 v_Position;

void main()
{
	o_Color = textureLod(u_Texture, v_Position, u_TextureLOD) * u_Intensity;
}
