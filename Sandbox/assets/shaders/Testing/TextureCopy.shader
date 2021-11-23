#type compute
#version 450 core

layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

uniform sampler2D u_ReadTexture;
uniform float u_LOD;

layout(local_size_x = 4, local_size_y = 4) in;
void main()
{
	vec2 readImageSize = vec2(imageSize(o_Image));
	ivec2 invocID = ivec2(gl_GlobalInvocationID);
	vec2 texCoords = vec2(float(invocID.x) / readImageSize.x, float(invocID.y) / readImageSize.y);

	vec3 color = textureLod(u_ReadTexture, texCoords, u_LOD).rgb;

	imageStore(o_Image, ivec2(gl_GlobalInvocationID), vec4(color, 1.0));
}