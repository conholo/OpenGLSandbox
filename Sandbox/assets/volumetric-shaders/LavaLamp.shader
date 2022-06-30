#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;

out vec3 v_ViewDirection;
out vec2 v_TexCoord;

void main()
{
	vec3 viewPosition = (u_InverseProjection * vec4(a_TexCoord * 2.0 - 1.0, 0.0, 1.0)).xyz;
	v_ViewDirection = (u_InverseView * vec4(viewPosition, 0.0)).xyz;

	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
}



#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

#define PI 3.14159

struct Light
{
	vec3 Position;
	float Intensity;
	vec3 Color;
};

uniform Light u_Light;

uniform sampler2D u_SceneTexture;
uniform sampler2D u_DepthTexture;

uniform int u_DensitySteps;
uniform int u_LightSteps;

uniform vec3 u_AbsorptionCoefficient;
uniform vec3 u_ScatteringCoefficient;

uniform float u_AtmosphereDensityFalloff;

uniform vec3 u_SphereAPosition;
uniform float u_SphereARadius;

uniform float u_NearClip;
uniform float u_FarClip;
uniform vec3 u_CameraPosition;

in vec2 v_TexCoord;
in vec3 v_ViewDirection;

// Based on https://64.github.io/tonemapping/
vec3 ACESTonemap(vec3 color)
{
	mat3 m1 = mat3(
		0.59719, 0.07600, 0.02840,
		0.35458, 0.90834, 0.13383,
		0.04823, 0.01566, 0.83777
	);
	mat3 m2 = mat3(
		1.60475, -0.10208, -0.00327,
		-0.53108, 1.10813, -0.07276,
		-0.07367, -0.00605, 1.07602
	);
	vec3 v = m1 * color;
	vec3 a = v * (v + 0.0245786) - 0.000090537;
	vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
	return clamp(m2 * (a / b), 0.0, 1.0);
}

vec3 GammaCorrect(vec3 color, float gamma)
{
	return pow(color, vec3(1.0f / gamma));
}

float CalculateLinearDepth()
{
	float depth = texture(u_DepthTexture, v_TexCoord).r;

	float near = u_NearClip;
	float far = u_FarClip;

	float ndcDepth = depth * 2.0 - 1.0;
	float linearDepth = (2.0 * near * far) / (far + near - ndcDepth * (far - near));

	return linearDepth;
}

struct SphereIntersectData
{
	float DistanceA;
	float DistanceB;
};
SphereIntersectData IntersectionBehind(vec3 p, vec3 d, vec3 c, float r)
{
	SphereIntersectData data;
	data.DistanceA = -1.0;
	data.DistanceB = -1.0;

	vec3 vpc = c - p;
	float vpcMag = length(vpc);

	// In front of sphere - intersections are behind us.
	if (vpcMag > r)
		return data;
	// We're on the perimeter of the sphere - radius is intersection.
	else if (vpcMag == r)
	{
		data.DistanceA = data.DistanceB = r;
		return data;
	}
	else
	{
		// Inside sphere, center is behind - project center onto line.
		float vpcDotD = dot(vpc, d);
		float dirSqrMag = dot(d, d);
		vec3 pud = p + vpcDotD / dirSqrMag * d;
		float pudToCMag = length(pud - c);
		float distToIntersectionFromPud = sqrt(r * r - pudToCMag * pudToCMag);
		float distToIntersectionFromP = distToIntersectionFromPud - length(pud - p);
		data.DistanceA = 0.0;
		data.DistanceB = distToIntersectionFromP;
		return data;
	}
}
SphereIntersectData IntersectionForward(vec3 p, vec3 d, vec3 c, float r)
{
	SphereIntersectData data;
	data.DistanceA = -1.0;
	data.DistanceB = -1.0;

	vec3 vpc = c - p;
	float vpcMag = length(vpc);

	float vpcDotD = dot(vpc, d);
	float dirSquareMag = dot(d, d);
	vec3 pud = p + vpcDotD / dirSquareMag * d;

	// No intersection - projection is greater than the radius
	if (length(c - pud) > r)
		return data;

	float pudToCMag = length(pud - c);
	float distanceFromPudToIntersection = sqrt(r * r - pudToCMag * pudToCMag);
	float pudToP = length(pud - p);

	if (vpcMag > r)
	{
		// Outside - Forward
		data.DistanceA = pudToP - distanceFromPudToIntersection;
		data.DistanceB = pudToP + distanceFromPudToIntersection;
	}
	else
	{
		// Inside - Forward
		data.DistanceA = 0.0;
		data.DistanceB = pudToP + distanceFromPudToIntersection;
	}

	return data;
}
SphereIntersectData RaySphereIntersect(vec3 p, vec3 d, vec3 c, float r)
{
	SphereIntersectData data;
	data.DistanceA = -1.0;
	data.DistanceB = -1.0;
	vec3 vpc = c - p;
	float vpcMag = length(vpc);

	// Sphere center is behind p.
	if (dot(vpc, d) < 0.0)
		return IntersectionBehind(p, d, c, r);
	// Sphere center projects onto line (p is behind center).
	return IntersectionForward(p, d, c, r);
}

// s_S - Out-Scattering Coefficient
// s_A - Absorption Coefficient
// s_T = s_S + s_A
// Medium Albedo = s_S / s_T [0, 1]

// Transmittance = e(-d * s_T)


float SampleDensityAtPosition(vec3 p, vec3 c, float r)
{
	float d = length(p - c);
	float heightPercent = d / r;
	return 1.0 - heightPercent;
}

float RayleighPhaseFunction(float a)
{
	float k = 3.0 / (16.0 * PI);
	return k * (1.0 * a * a);
}

float CalculateOpticalDepth(vec3 ro, vec3 rd, float rayLength)
{
	float stepSize = rayLength / float(u_DensitySteps);
	float opticalDepth = 0.0;
	float distanceTraveled = 0.0;

	for (int i = 0; i < u_DensitySteps; i++)
	{
		vec3 scatterPoint = ro + rd * distanceTraveled;
		float density = SampleDensityAtPosition(scatterPoint, u_SphereAPosition, u_SphereARadius);
		opticalDepth += density * stepSize;
		distanceTraveled += stepSize;
	}

	return opticalDepth;
}

void main()
{
	vec3 sceneColor = texture(u_SceneTexture, v_TexCoord).rgb;

	vec3 rd = normalize(v_ViewDirection);
	vec3 ro = u_CameraPosition;
	vec3 ld = normalize(u_Light.Position - u_CameraPosition);

	float linearDepth = CalculateLinearDepth();
	SphereIntersectData sphereIntersect = RaySphereIntersect(ro, rd, u_SphereAPosition, u_SphereARadius);

	bool hitSphere = (sphereIntersect.DistanceA + sphereIntersect.DistanceB) > 0.0;
	bool depthAccurate = sphereIntersect.DistanceA < linearDepth;

	vec3 volumetricLighting = vec3(0.0);

	float volumeDepth = sphereIntersect.DistanceB - sphereIntersect.DistanceA;
	float stepSize = volumeDepth / float(u_DensitySteps);
	float distanceTraveled = 0.0;

	vec3 sigmaT = u_AbsorptionCoefficient + u_ScatteringCoefficient;

	vec3 intersectionPoint = ro + rd * sphereIntersect.DistanceA;
	float totalDensity = 0.0;

	for (int i = 0; i < u_DensitySteps; i++)
	{
		vec3 samplePosition = intersectionPoint + rd * distanceTraveled;
		float densityAtSample = SampleDensityAtPosition(samplePosition, u_SphereAPosition, u_SphereARadius);

		if (densityAtSample > 0.0)
		{
			vec3 directionToLight = normalize(u_Light.Position - samplePosition);
			SphereIntersectData sunSphereIntersectData = RaySphereIntersect(samplePosition, directionToLight, u_SphereAPosition, u_SphereARadius);
			float distanceToSun = sunSphereIntersectData.DistanceB - sunSphereIntersectData.DistanceA;
			float sunRayOpticalDepth = CalculateOpticalDepth(samplePosition, directionToLight, distanceToSun);

			float phase = RayleighPhaseFunction(dot(rd, directionToLight));
			float viewRayOpticalDepth = CalculateOpticalDepth(samplePosition, -rd, distanceTraveled) * phase;

			vec3 atmosphereTransmittance = exp(-(sunRayOpticalDepth + viewRayOpticalDepth) * sigmaT);
			volumetricLighting += atmosphereTransmittance * densityAtSample * stepSize * u_ScatteringCoefficient;
		}

		distanceTraveled += stepSize;
	}

	sceneColor = sceneColor + volumetricLighting;

	vec3 result = ACESTonemap(sceneColor);
	result = GammaCorrect(result, 2.2);
	o_Color = vec4(sceneColor, 1.0);
}