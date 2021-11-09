#type compute
#version 450 core

layout(rgba32f, binding = 0) restrict writeonly uniform imageCube o_SkyMap;

//https://en.wikipedia.org/wiki/Cube_mapping
vec3 GetCubeMapTexCoord()
{
	vec2 st = gl_GlobalInvocationID.xy / vec2(imageSize(o_SkyMap));
	vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

	vec3 coords;

	if		(gl_GlobalInvocationID.z == 0)	coords = vec3(1.0, uv.y, -uv.x);  // +X
	else if (gl_GlobalInvocationID.z == 1)	coords = vec3(-1.0, uv.y, uv.x);  // -X
	else if (gl_GlobalInvocationID.z == 2)	coords = vec3(uv.x, 1.0, -uv.y);  // +Y
	else if (gl_GlobalInvocationID.z == 3)	coords = vec3(uv.x, -1.0, uv.y);  // -Y
	else if (gl_GlobalInvocationID.z == 4)	coords = vec3(uv.x, uv.y, 1.0);   // +Z
	else if (gl_GlobalInvocationID.z == 5)	coords = vec3(-uv.x, uv.y, -1.0); // -Z

	return normalize(coords);
}

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	vec3 textureCoords = GetCubeMapTexCoord();

	vec4 color = vec4(textureCoords, 1.0);
	imageStore(o_SkyMap, ivec3(gl_GlobalInvocationID), color);
}