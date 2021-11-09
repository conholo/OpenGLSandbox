#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

out mat4 v_ViewMatrix;
out mat4 v_ProjectionMatrix;

out vec3 v_NearPoint;
out vec3 v_FarPoint;

void main()
{
	vec3 position = a_Position;
	position.x += sign(a_Position.x) * 0.5;
	position.y += sign(a_Position.y) * 0.5;

	v_ViewMatrix = u_ViewMatrix;
	v_ProjectionMatrix = u_ProjectionMatrix;

	vec4 nearUnprojected = (inverse(u_ViewMatrix) * inverse(u_ProjectionMatrix) * vec4(position.x, position.y, 0.0, 1.0));
	v_NearPoint = nearUnprojected.xyz / nearUnprojected.w;
	vec4 farUnprojected = (inverse(u_ViewMatrix) * inverse(u_ProjectionMatrix) * vec4(position.x, position.y, 1.0, 1.0));
	v_FarPoint = farUnprojected.xyz / farUnprojected.w;

	gl_Position = vec4(position, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;
uniform vec3 u_Color;

in vec3 v_NearPoint;
in vec3 v_FarPoint;
in mat4 v_ViewMatrix;
in mat4 v_ProjectionMatrix;

float ComputeDepth(vec3 position)
{
	vec4 clipSpacePosition = v_ProjectionMatrix * v_ViewMatrix * vec4(position.xyz, 1.0);
	return clipSpacePosition.z / clipSpacePosition.w;
}

void main()
{
	float t = -v_NearPoint.y / (v_FarPoint.y - v_NearPoint.y);
	vec3 fragPosition = v_NearPoint + t * (v_FarPoint - v_NearPoint);
	
	gl_FragDepth = ComputeDepth(fragPosition);

	o_Color = vec4(vec3(gl_FragDepth), 1.0) * float(t > 0);
}
