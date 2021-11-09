#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;


uniform mat4 u_InverseViewMatrix;
uniform mat4 u_InverseProjectionMatrix;

out vec3 v_NearPoint;
out vec3 v_FarPoint;

vec3 UnprojectPoint(float x, float y, float z)
{
	vec4 unprojectedPoint = u_InverseViewMatrix * u_InverseProjectionMatrix * vec4(x, y, z, 1.0);
	return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main()
{
	gl_Position = view.proj * view.view * vec4(gridPlane[gl_VertexIndex].xyz, 1.0);
}


#type fragment

#version 450 core

layout(location = 0) out vec4 o_Color;

void main()
{
	o_Color = vec4(1.0, 1.0, 1.0, 1.0);
}