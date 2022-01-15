#type compute
#version 460 core

layout(std140, binding = 0) buffer ValueBuffer
{
	vec4 Values[];
};

uniform vec2 u_Dimensions;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
	ivec2 id = ivec2(gl_GlobalInvocationID.xy);

	vec2 texCoord = vec2(id) / (u_Dimensions - 1.0);
	int index = id.x + id.y * int(u_Dimensions.x);
	Values[index].xy = vec2(index, 0.0);
}