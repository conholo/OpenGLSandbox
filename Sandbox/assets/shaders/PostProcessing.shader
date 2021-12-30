#type vertex
#version 450 core

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

uniform sampler2D u_Texture;
uniform sampler2D u_BloomTexture;
uniform sampler2D u_BloomDirtTexture;
uniform float u_BloomDirtIntensity;
uniform float u_BloomIntensity;
uniform float u_Exposure;

in vec2 v_TexCoord;

vec3 UpsampleTent9(sampler2D tex, float lod, vec2 uv, vec2 texelSize, float scale)
{
	vec4 offset = texelSize.xyxy * vec4(1.0f, 1.0f, -1.0f, 0.0f) * scale;

	// Center
	vec3 result = textureLod(tex, uv, lod).rgb * 4.0f;

	result += textureLod(tex, uv - offset.xy, lod).rgb;
	result += textureLod(tex, uv - offset.wy, lod).rgb * 2.0;
	result += textureLod(tex, uv - offset.zy, lod).rgb;

	result += textureLod(tex, uv + offset.zw, lod).rgb * 2.0;
	result += textureLod(tex, uv + offset.xw, lod).rgb * 2.0;

	result += textureLod(tex, uv + offset.zy, lod).rgb;
	result += textureLod(tex, uv + offset.wy, lod).rgb * 2.0;
	result += textureLod(tex, uv + offset.xy, lod).rgb;

	return result * (1.0f / 16.0f);
}

// Based on https://64.github.io/tonemapping/
vec3 ACESTonemap(vec3 color)
{
	mat3 m1 = mat3(
		0.59719, 0.07600, 0.02840,
		0.35458, 0.90834, 0.13383,
		0.04823, 0.01566, 0.83777
	);
	mat3 m2 = mat3(
		1.60475, -0.10208, -0.00327,
		-0.53108, 1.10813, -0.07276,
		-0.07367, -0.00605, 1.07602
	);
	vec3 v = m1 * color;
	vec3 a = v * (v + 0.0245786) - 0.000090537;
	vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
	return clamp(m2 * (a / b), 0.0, 1.0);
}

vec3 GammaCorrect(vec3 color, float gamma)
{
	return pow(color, vec3(1.0f / gamma));
}

void main()
{
	const float gamma = 2.2;
	float sampleScale = 1.0;

	ivec2 texSize = textureSize(u_BloomTexture, 0);
	vec2 fTexSize = vec2(float(texSize.x), float(texSize.y));
	vec3 bloom = max(UpsampleTent9(u_BloomTexture, 0, v_TexCoord, 1.0f / fTexSize, sampleScale) * u_BloomIntensity, vec3(0.0));
	vec3 bloomDirt = texture(u_BloomDirtTexture, v_TexCoord).rgb * u_BloomDirtIntensity;

	vec3 color = texture(u_Texture, v_TexCoord).rgb;
	color += bloom;
	color += bloom * bloomDirt;
	color *= u_Exposure;

	color = ACESTonemap(color.rgb);
	color = GammaCorrect(color.rgb, gamma);
	o_Color = vec4(color, 1.0);
}