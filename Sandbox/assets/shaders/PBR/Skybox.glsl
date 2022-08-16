#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 0) out vec3 v_Position;

uniform mat4 u_ViewProjection;

void main()
{
	vec4 position = u_ViewProjection * vec4(a_Position, 1.0);
	gl_Position = position.xyww;

	v_Position = vec3(a_Position.x, a_Position.y, -a_Position.z);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Color2;
layout(location = 0) in vec3 v_Position;
uniform samplerCube u_Texture;

uniform float u_TextureLOD;
uniform float u_Intensity;

void main()
{
	o_Color = textureLod(u_Texture, v_Position, u_TextureLOD) * u_Intensity;
	o_Color.a = 1.0;
	o_Color2 = vec4(0.0);
}
