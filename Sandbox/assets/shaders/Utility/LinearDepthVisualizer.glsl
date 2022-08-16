#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_DepthVis;

uniform sampler2D u_DepthTexture;
uniform float u_Near;
uniform float u_Far;

in vec2 v_TexCoord;

float LinearizeDepth(float depth)
{
	float near = u_Near;
	float far = u_Far;

	float ndc = depth * 2.0 - 1.0;
	return ((2.0 * near * far) / (far + near - ndc * (far - near))) / far;
}

void main()
{
	float depth = texture(u_DepthTexture, v_TexCoord).r;
	float linearDepth = LinearizeDepth(depth);
	o_DepthVis = vec4(vec3(linearDepth), 1.0f);
}
