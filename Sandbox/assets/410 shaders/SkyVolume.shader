#type compute
#version 450 core

layout(rgba32f, binding = 0) restrict writeonly uniform imageCube o_SkyMap;
#define PI 3.14159265359

uniform vec3 u_TAI;

vec3 CubeMapTexCoords()
{
	vec2 st = gl_GlobalInvocationID.xy / vec2(imageSize(o_SkyMap));

	// flip y
	vec2 uv = vec2(st.x, 1.0 - st.y) * 2.0 - 1.0;

	vec3 coords;

	if		(gl_GlobalInvocationID.z == 0) coords = vec3( 1.0, uv.y, -uv.x);
	else if (gl_GlobalInvocationID.z == 1) coords = vec3( -1.0, uv.y, uv.x);
	else if (gl_GlobalInvocationID.z == 2) coords = vec3( uv.x, 1.0, -uv.y);
	else if (gl_GlobalInvocationID.z == 3) coords = vec3( uv.x, -1.0, uv.y);
	else if (gl_GlobalInvocationID.z == 4) coords = vec3( uv.x, uv.y, 1.0 );
	else if (gl_GlobalInvocationID.z == 5) coords = vec3(-uv.x, uv.y, -1.0);

	return normalize(coords);
}

float SaturatedDot(in vec3 a, in vec3 b)
{
	return max(dot(a, b), 0.0);
}

vec3 CalculateZenithLuminanceYxy(in float t, in float thetaS)
{
	float chi = (4.0 / 9.0 - t / 120.0) * (PI - 2.0 * thetaS);
	float Yz = (4.0453 * t - 4.9710) * tan(chi) - 0.2155 * t + 2.4192;

	float theta2 = thetaS * thetaS;
	float theta3 = theta2 * thetaS;
	float T = t;
	float T2 = t * t;

	float xz =
		(0.00165 * theta3 - 0.00375 * theta2 + 0.00209 * thetaS + 0.0) * T2 +
		(-0.02903 * theta3 + 0.06377 * theta2 - 0.03202 * thetaS + 0.00394) * T +
		(0.11693 * theta3 - 0.21196 * theta2 + 0.06052 * thetaS + 0.25886);

	float yz =
		(0.00275 * theta3 - 0.00610 * theta2 + 0.00317 * thetaS + 0.0) * T2 +
		(-0.04214 * theta3 + 0.08970 * theta2 - 0.04153 * thetaS + 0.00516) * T +
		(0.15346 * theta3 - 0.26756 * theta2 + 0.06670 * thetaS + 0.26688);

	return vec3(Yz, xz, yz);
}

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	vec3 textureCoords = CubeMapTexCoords();

	float turbidity = u_TAI.x;
	float azimuth = u_TAI.y;
	float inclination = u_TAI.z;

	vec3 s = normalize(vec3(sin(inclination) * cos(azimuth), cos(inclination), sin(inclination) * sin(azimuth)));
	vec3 v = textureCoords;

	float thetaS = acos(SaturatedDot(s, vec3(0, 1, 0)));
	vec3 Yz = CalculateZenithLuminanceYxy(turbidity, thetaS);

	float t = dot(textureCoords, vec3(0.0f, 1.0f, 0.0f));

	vec3 color = mix(vec3(0.2f, 0.1f, 0.4f), vec3(0.5f, 0.7f, 0.9f), t) + Yz;
	imageStore(o_SkyMap, ivec3(gl_GlobalInvocationID), vec4(color, 1.0));
}