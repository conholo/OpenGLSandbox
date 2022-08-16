#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_MVP;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;

out vec3 v_WorldSpaceViewDirection;

void main()
{
	vec3 viewVector = (u_InverseProjection * vec4(a_TexCoord * 2.0 - 1.0, 0.0, 1.0)).xyz;
	v_WorldSpaceViewDirection = (u_InverseView * vec4(viewVector, 0.0)).xyz;

	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;
#define PI 3.14159265359
#define EPSILON 0.0001

struct Sun
{
	vec3 Position;
	float Intensity;
	vec3 Color;
};
uniform Sun u_Sun;

uniform vec2 u_ScreenResolution;
uniform vec3 u_CameraPosition;

uniform int u_Steps;
uniform int u_Octaves;

uniform float u_SeaChoppy;
uniform float u_SeaAmplitude;
uniform float u_SeaFrequency;
uniform float u_AnimationTime;
uniform float u_SeaHeight;

uniform float u_NearClip;
uniform float u_FarClip;

in vec3 v_WorldSpaceViewDirection;

float hash(vec2 p)
{
	float h = dot(p, vec2(127.1, 311.7));
	return fract(sin(h) * 43758.5453123);
}

float noise(in vec2 p)
{
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);
	return -1.0 + 2.0 * mix(mix(hash(i + vec2(0.0, 0.0)),
		hash(i + vec2(1.0, 0.0)), u.x),
		mix(hash(i + vec2(0.0, 1.0)),
			hash(i + vec2(1.0, 1.0)), u.x), u.y);
}


vec3 CalculateSkyColor(vec3 e)
{
	e.y = (max(e.y, 0.0) * 0.8 + 0.2) * 0.8;
	return vec3(pow(1.0 - e.y, 2.0), 1.0 - e.y, 0.6 + (1.0 - e.y) * 0.4) * 1.1;
}

float SeaNoise(vec2 uv, float choppyConstant)
{
	uv += noise(uv);
	vec2 wv = 1.0 - abs(sin(uv));
	vec2 swv = abs(cos(uv));
	wv = mix(wv, swv, wv);
	return pow(1.0 - pow(wv.x * wv.y, 0.65), choppyConstant);
}

float Map(vec3 p)
{
	float freq = u_SeaFrequency;
	float amp = u_SeaAmplitude;
	float choppy = u_SeaChoppy;
	vec2 uv = p.xz;
	const mat2 octaveM = mat2(1.6, 1.2, -1.2, 1.6);

	float d, h = 0.0;

	for (int i = 0; i < u_Octaves; i++)
	{
		d = SeaNoise((uv + u_AnimationTime) * freq, choppy);
		d += SeaNoise((uv - u_AnimationTime) * freq, choppy);
		h += d * amp;
		uv *= octaveM;
		freq *= 1.9;
		amp *= 0.22;
		choppy = mix(choppy, 1.0, 0.2);
	}

	return p.y - h;
}
#define SEA_BASE vec3(0.0, 0.09, 0.18)
#define SEA_WATER_COLOR vec3(0.8, 0.9, 0.6) * 0.6
#define FOV 60
#define EPSILON_NRM (0.1 / u_ScreenResolution.x)


float CalculateDiffuse(vec3 n, vec3 l, float p)
{
	return pow(dot(n, l) * 0.4 + 0.6, p);
}

float CalculateSpecular(vec3 n, vec3 l, vec3 v, float s)
{
	float nrm = (s + 8.0) / (PI * 8.0);
	return pow(max(0.0, dot(reflect(v, n), l)), s) * nrm;
}

vec3 CalculateSeaColor(vec3 p, vec3 n, vec3 l, vec3 v, vec3 dist)
{
	float fresnel = clamp(1.0 - dot(n, -v), 0.0, 1.0);
	fresnel = pow(fresnel, 3.0) * 0.5;

	vec3 reflected = CalculateSkyColor(reflect(v, n));
	vec3 refracted = SEA_BASE + CalculateDiffuse(n, l, 80.0) * SEA_WATER_COLOR * 0.12;

	vec3 color = mix(refracted, reflected, fresnel);

	float attenuation = max(1.0 - dot(dist, dist) * 0.001, 0.0);
	color += SEA_WATER_COLOR * (p.y - u_SeaHeight) * 0.18 * attenuation;
	color += vec3(CalculateSpecular(n, l, v, 60.0));

	return color;
}

vec3 CalculateNormal(vec3 p, float eps)
{
	vec3 normal;
	normal.y = Map(p);
	normal.x = Map(vec3(p.x + eps, p.y, p.z)) - normal.y;
	normal.z = Map(vec3(p.x, p.y, p.z + eps)) - normal.y;
	normal.y = eps;
	return normalize(normal);
}

vec3 TraceHeightMap(vec3 ro, vec3 rd)
{
	vec3 p = vec3(0.0);
	float tm = u_NearClip;
	float tx = 5000;
	float hx = Map(ro + rd * tx);
	if (hx > 0.0)
	{
		p = ro + rd * tx;
		return p;
	}

	float hm = Map(ro + rd * tm);
	float tmid = 0.0;

	for (int i = 0; i < u_Steps; i++)
	{
		tmid = mix(tm, tx, hm / (hm - hx));
		p = ro + rd * tmid;
		float hmid = Map(p);

		if (hmid < 0.0)
		{
			tx = tmid;
			hx = hmid;
		}
		else
		{
			tm = tmid;
			hm = hmid;
		}
	}

	return p;
}


void main()
{
	vec3 ro = u_CameraPosition;
	vec3 rd = normalize(v_WorldSpaceViewDirection);

	vec3 p = TraceHeightMap(ro, rd);
	vec3 d = p - ro;
	vec3 n = CalculateNormal(p, dot(d, d) * EPSILON_NRM);
	vec3 l = normalize(u_Sun.Position - p);

	vec3 color = mix(CalculateSkyColor(rd), CalculateSeaColor(p, n, l, rd, d), pow(smoothstep(0.0, -0.02, rd.y), 0.2));
	
	o_Color = vec4(pow(color, vec3(0.99)), 1.0);
}