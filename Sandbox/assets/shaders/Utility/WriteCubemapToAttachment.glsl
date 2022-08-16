#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

out vec3 v_WorldSpaceViewDirection;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;

void main()
{
	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
	
    vec3 viewVector = (u_InverseProjection * vec4(a_TexCoord * 2.0 - 1.0, 0.0, 1.0)).xyz;
	v_WorldSpaceViewDirection = (u_InverseView * vec4(viewVector, 0.0)).xyz;
}


#type fragment
#version 450
layout(location = 0) out vec4 o_Color;

uniform samplerCube u_RadianceMap;
in vec3 v_WorldSpaceViewDirection;

void main()
{
    vec3 ViewDirection = normalize(v_WorldSpaceViewDirection);
    vec3 R1 = texture(u_RadianceMap, ViewDirection).rgb;
    o_Color = vec4(R1, 1.0);
}