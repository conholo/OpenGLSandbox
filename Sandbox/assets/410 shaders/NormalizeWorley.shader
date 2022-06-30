#type compute
#version 450 core

#define MIN_MAX_ACCURACY 10000000

layout(binding = 0, rgba32f) uniform image3D o_Image;

uniform vec4 u_ChannelMask;

layout(std430, binding = 0) buffer MinMaxBuffer
{
	int MinMax[2];
};

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main()
{
	float minValue = float(MinMax[0]) / float(MIN_MAX_ACCURACY);
	float maxValue = float(MinMax[1]) / float(MIN_MAX_ACCURACY);

	vec4 current = imageLoad(o_Image, ivec3(gl_GlobalInvocationID));
	vec4 normalized = (current - minValue) / (maxValue - minValue);

	vec4 result = current * (1.0 - u_ChannelMask) + normalized * u_ChannelMask;
	imageStore(o_Image, ivec3(gl_GlobalInvocationID), result);
}
