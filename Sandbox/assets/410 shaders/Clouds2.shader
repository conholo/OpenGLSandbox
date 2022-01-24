#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;


uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;

out vec2 v_TexCoord;

void main()
{
	vec3 viewDirection = (u_InverseProjection * vec4(a_TexCoord * 2.0 - 1.0, 0.0, 1.0)).xyz;
	v_ViewDirection = (u_InverseView * vec4(viewDirection, 0.0));
	v_TexCoord = a_TexCoord;

	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;
in vec3 v_ViewDirection;

uniform sampler2D u_HeightMap;
uniform sampler3D u_ShapeTexture;
uniform sampler3D u_DetailTexture;
uniform sampler2D u_CurlTexture;

void main()
{
	vec4 color = vec4(0.0);

	// Calculate Density 
		// From the camera in the direction of this fragment
		// 
}

