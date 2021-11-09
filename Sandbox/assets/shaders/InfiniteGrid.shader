#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_InverseProjectionMatrix;
uniform mat4 u_InverseViewMatrix;

out vec3 v_NearPoint;
out vec3 v_FarPoint;

vec3 UnprojectPoint(float x, float y, float z)
{
	vec4 unprojected = u_InverseViewMatrix * u_InverseProjectionMatrix * vec4(x, y, z, 1.0);
	return unprojected.xyz / unprojected.w;
}

void main()
{
	v_NearPoint = UnprojectPoint(a_Position.x, a_Position.y, 0.0).xyz;
	v_FarPoint = UnprojectPoint(a_Position.x, a_Position.y, 1.0).xyz;
	gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

in vec3 v_NearPoint;
in vec3 v_FarPoint;

uniform float u_NearClip;
uniform float u_FarClip;
uniform float u_InnerGridScale;
uniform float u_OuterGridScale;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

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
	vec4 clipSpacePosition = u_ProjectionMatrix * u_ViewMatrix * vec4(position.xyz, 1.0);
	return clipSpacePosition.z / clipSpacePosition.w;
}

float ComputeLinearDepth(vec3 position)
{	
	vec4 clipSpacePosition = u_ProjectionMatrix * u_ViewMatrix * vec4(position.xyz, 1.0);
	float clipSpaceDepth = (clipSpacePosition.z / clipSpacePosition.w) * 2.0 - 1.0;
	float linearDepth = (2.0 * u_NearClip * u_FarClip) / (u_FarClip + u_NearClip - clipSpaceDepth * (u_FarClip - u_NearClip));

	return linearDepth / u_FarClip;
}

void main()
{
	float t = -v_NearPoint.y / (v_FarPoint.y - v_NearPoint.y);
	vec3 fragPosition = v_NearPoint + t * (v_FarPoint - v_NearPoint);

	gl_FragDepth = ComputeDepth(fragPosition);

	float linearDepth = ComputeLinearDepth(fragPosition);
	float fade = max(0, 0.5 - linearDepth);

	o_Color = (Grid(fragPosition, u_InnerGridScale, true) + Grid(fragPosition, u_OuterGridScale, true)) * float(t > 0);
	o_Color.a *= fade;
}