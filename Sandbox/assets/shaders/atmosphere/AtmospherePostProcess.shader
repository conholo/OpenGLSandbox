#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

uniform sampler2D u_HDRBuffer;

float sRGB(float x)
{
	if (x <= 0.00031308)
		return 12.92 * x;
	else
		return 1.055 * pow(x, (1.0 / 2.4)) - 0.055;
}

vec4 sRGB(vec4 vec)
{
	return vec4(sRGB(vec.x), sRGB(vec.y), sRGB(vec.z), vec.w);
}

void main()
{
	vec4 rgba = texture(u_HDRBuffer, v_TexCoord);
	rgba /= rgba.aaaa;	// Normalize according to sample count when path tracing

	// Similar setup to the Bruneton demo
	vec3 white_point = vec3(1.08241, 0.96756, 0.95003);
	float exposure = 10.0;

	vec3 termA = vec3(1.0) - exp(-rgba.rgb / white_point * exposure);
	vec3 gammaCorrect = vec3(1.0 / 2.2);
	vec3 gammaCorrected = pow(termA, gammaCorrect);

	o_Color = vec4(gammaCorrected, 1.0);
}