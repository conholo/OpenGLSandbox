#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_InverseProjectionMatrix;
uniform mat4 u_InverseViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform float u_NearClip;
uniform float u_FarClip;

out vec3 v_NearPoint;
out vec3 v_FarPoint;
out mat4 v_ProjectionMatrix;
out mat4 v_ViewMatrix;
out float v_NearClip;
out float v_FarClip;

vec3 UnprojectPoint(float x, float y, float z)
{
	vec4 unprojected = u_InverseViewMatrix * u_InverseProjectionMatrix * vec4(x, y, z, 1.0);
	return unprojected.xyz / unprojected.w;
}

void main()
{
	v_NearClip = u_NearClip;
	v_FarClip = u_FarClip;

	v_ProjectionMatrix = u_ProjectionMatrix;
	v_ViewMatrix = u_ViewMatrix;
	v_NearPoint = UnprojectPoint(a_Position.x, a_Position.y, 0.0).xyz;
	v_FarPoint = UnprojectPoint(a_Position.x, a_Position.y, 1.0).xyz;

	gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;
#define PI 3.14159265359

in vec3 v_NearPoint;
in vec3 v_FarPoint;

in mat4 v_ProjectionMatrix;
in mat4 v_ViewMatrix;
in float v_NearClip;
in float v_FarClip;

uniform float u_ElapsedTime;
uniform float u_Tiling;
uniform float u_Speed;
uniform vec2 u_Offset;
uniform sampler2D u_WaterTexture;
uniform sampler2D u_DepthTexture;

float MultiWave(float x, float z, float t)
{
	float y = sin(PI * (x + 0.5 * t));
	y += 0.5 * sin(2.0 * PI * (z + t));
	y += sin(PI * (x + z + 0.25 * t));

	return y * (1.0 / 2.5);
}

vec4 Water(vec3 fragPosition, float depth)
{
	float foam = depth / v_NearClip;
	vec2 samplePosition = fragPosition.xz + u_Offset * u_Speed * u_ElapsedTime;
	vec3 water = texture(u_WaterTexture, samplePosition * u_Tiling).r * vec3(0.0, 0.2f, 0.7f) + vec3(0.0, 0.3f, 0.9f) * foam;
	water.b -= 1.0;
	return vec4(water, 1.0);
}

float ComputeDepth(vec3 position)
{
	float near = gl_DepthRange.near;
	float far = gl_DepthRange.far;
	vec4 clipSpacePosition = v_ProjectionMatrix * v_ViewMatrix * vec4(position.xyz, 1.0);
	float ndcDepth = clipSpacePosition.z / clipSpacePosition.w;
	return (((far - near) * ndcDepth) + near + far) / 2.0;
}

float ComputeLinearDepth(vec3 position)
{
	float near = gl_DepthRange.near;
	float far = gl_DepthRange.far;

	vec4 clipSpacePosition = v_ProjectionMatrix * v_ViewMatrix * vec4(position.xyz, 1.0);
	float clipSpaceDepth = (clipSpacePosition.z / clipSpacePosition.w) * 2.0 - 1.0;
	float linearDepth = (2.0 * near * far) / (far + near - clipSpaceDepth * (far - near));

	return linearDepth;
}

void main()
{
	float far = gl_DepthRange.far;
	float near = gl_DepthRange.near;

	float t0 = -v_NearPoint.y / (v_FarPoint.y - v_NearPoint.y);
	vec3 p = v_NearPoint + t0 * (v_FarPoint - v_NearPoint);

	float wave = MultiWave(p.x + u_Offset.x, p.z + u_Offset.y, u_ElapsedTime) * 5.0;
	float t = (-v_NearPoint.y + wave) / (v_FarPoint.y - v_NearPoint.y);
	vec3 fragPosition = v_NearPoint + t * (v_FarPoint - v_NearPoint);
	gl_FragDepth = ComputeDepth(fragPosition);

	float linearDepth = ComputeLinearDepth(fragPosition);
	float fade = max(0, 0.5 - linearDepth / v_FarClip);
	o_Color = Water(fragPosition, linearDepth) * float(t > 0.0);
	o_Color.a *= fade;
}