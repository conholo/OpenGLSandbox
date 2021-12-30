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

uniform float u_InnerGridScale;
uniform float u_OuterGridScale;

in vec3 v_NearPoint;
in vec3 v_FarPoint;

in mat4 v_ProjectionMatrix;
in mat4 v_ViewMatrix;
in float v_NearClip;
in float v_FarClip;

vec4 Grid(vec3 fragPosition, float scale, bool drawAxis)
{
	vec2 coord = fragPosition.xz * scale;
	vec2 derivative = fwidth(coord);
	vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;

	float linePos = min(grid.x, grid.y);
	float minimumX = min(derivative.x, 1);
	float minimumZ = min(derivative.y, 1);

	vec4 color = vec4(0.22, 0.22, 0.22, 1.0 - min(linePos, 1.0));

	color.z = drawAxis && fragPosition.x > -0.3 * minimumX && fragPosition.x < 0.3 * minimumX ? 1.0 : 0.22f;
	color.x = drawAxis && fragPosition.z > -0.3 * minimumZ && fragPosition.z < 0.3 * minimumZ ? 1.0 : 0.22f;

	return color;
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

	return linearDepth / v_FarClip;
}

void main()
{
	float far = gl_DepthRange.far;
	float near = gl_DepthRange.near;

	float t = -v_NearPoint.y / (v_FarPoint.y - v_NearPoint.y);
	vec3 fragPosition = v_NearPoint + t * (v_FarPoint - v_NearPoint);

	gl_FragDepth = ComputeDepth(fragPosition);

	float linearDepth = ComputeLinearDepth(fragPosition);
	float fade = max(0, 0.5 - linearDepth);
	o_Color = (Grid(fragPosition, u_OuterGridScale, true) + Grid(fragPosition, u_InnerGridScale, true)) * float(t > 0);
	o_Color.a *= fade;

	//o_Color = vec4(0.1, 0.0, 0.0, 1.0 * float(t > 0.0));
}