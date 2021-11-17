#type compute
#version 450 core

layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

uniform sampler2D u_Texture;

layout(local_size_x = 32, local_size_y = 32) in;
void main()
{
	vec2 imgSize = vec2(imageSize(o_Image));
	ivec2 invocID = ivec2(gl_GlobalInvocationID);
	vec2 texCoords = vec2(float(invocID.x) / imgSize.x, float(invocID.y) / imgSize.y);

	vec4 color = texture(u_Texture, texCoords);

	imageStore(o_Image, ivec2(gl_GlobalInvocationID), color);
}
