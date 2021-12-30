#type compute
#version 450 core

layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

uniform int u_Values[16];
uniform int u_ValueX;
uniform int u_ValueY;

layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
void main()
{
	bool isRed = gl_GlobalInvocationID.x == u_ValueX || gl_GlobalInvocationID.y == u_ValueY;
	imageStore(o_Image, ivec2(gl_GlobalInvocationID), isRed ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(1.0));
}