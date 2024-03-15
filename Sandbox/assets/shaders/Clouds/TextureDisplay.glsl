#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_Tangent;
layout(location = 3) in vec2 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 u_MVP;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

uniform vec2 u_ScreenResolution;
uniform vec2 u_PercentOfScreen;
uniform sampler2D u_Texture;

in vec2 v_TexCoord;

float InverseLerp(float from, float to, float value)
{
	return (value - from) / (to - from);
}

float Lerp(float from, float to, float value)
{
	return ((1.0 - value) * from) + (value * to);
}

float Remap(float originalFrom, float originalTo, float targetFrom, float targetTo, float value)
{
	float t = InverseLerp(originalFrom, originalTo, value);
	return Lerp(targetFrom, targetTo, t);
}

void main()
{
	vec2 pctScreen = gl_FragCoord.xy / u_ScreenResolution;

	vec2 bottomLeftTexCoords = u_ScreenResolution - u_ScreenResolution * u_PercentOfScreen;

	if (gl_FragCoord.x < bottomLeftTexCoords.x || gl_FragCoord.y < bottomLeftTexCoords.y)
		discard;

	vec2 percentCoord = gl_FragCoord.xy / u_ScreenResolution;

	vec2 minExtents = 1.0 - u_PercentOfScreen;

	float texX = Remap(minExtents.x, 1.0, 0.0, 1.0, percentCoord.x);
	float texY = Remap(minExtents.y, 1.0, 0.0, 1.0, percentCoord.y);
	
	vec2 coord = vec2(texX, texY);

	o_Color = texture(u_Texture, coord);
}
