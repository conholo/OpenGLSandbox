#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

out vec3 v_ViewDirection;
out vec2 v_TexCoord;
out vec3 v_WorldCoord;

uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;

void main()
{
	vec3 viewPosition = (u_InverseProjection * vec4(a_TexCoord * 2.0 - 1.0, 0.0, 1.0)).xyz;
	v_ViewDirection = (u_InverseView * vec4(viewPosition, 0.0)).xyz;
	v_WorldCoord = (u_InverseView * vec4(viewPosition, 1.0)).xyz;
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;
in vec3 v_ViewDirection;
in vec3 v_WorldCoord;

#define PI 3.14159

struct Light
{
	vec3 Position;
	vec3 Color;
	float Intensity;
};

uniform sampler2D u_SceneTexture;
uniform sampler2D u_DepthTexture;

uniform float u_NearClip;
uniform float u_FarClip;

uniform float u_ElapsedTime;
uniform float u_TimeScale;
uniform float u_AnimationSpeed;
uniform bool u_Animate;

uniform Light u_Light;

uniform int u_DensitySteps;
uniform int u_LightSteps;

uniform sampler3D u_ShapeTexture;
uniform float u_LowerCloudOffsetPct;
uniform float u_UpperCloudOffsetPct;
uniform float u_ViewerAttenuationFactor;
uniform float u_CloudScale;
uniform float u_DensityMultiplier;
uniform float u_DensityThreshold;
uniform float u_LuminanceMultiplier;
uniform vec4 u_ShapeNoiseWeights;

uniform float u_AtmosphereStrength;

uniform float u_AtmosphereRadius;
uniform float u_PlanetRadius;
uniform vec3 u_PlanetPosition;

uniform vec3 u_CameraPosition;

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


struct IntersectionData
{
	float DistanceA;
	float DistanceB;
	bool Hit;
	bool Outside;
	bool Forward;
	bool HitPlanet;
};

IntersectionData TestIntersectionBehind(vec3 p, vec3 d, vec3 c, float r)
{
	IntersectionData data;
	data.DistanceA = -1.0;
	data.DistanceB = -1.0;
	data.Hit = false;
	data.Outside = false;
	data.Forward = false;
	data.HitPlanet = false;

	vec3 vpc = c - p;
	float vpcMag = length(vpc);

	// In front of sphere - intersections are behind us.
	if (vpcMag > r)
		return data;
	// We're on the perimeter of the sphere - radius is intersection.
	else if (vpcMag == r)
	{
		data.Hit = true;
		data.Outside = true;
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

		data.Hit = true;
		data.DistanceA = 0.0;
		data.DistanceB = distToIntersectionFromP;
		return data;
	}
}

IntersectionData TestIntersectionForward(vec3 p, vec3 d, vec3 c, float r)
{
	IntersectionData data;
	data.DistanceA = -1.0;
	data.DistanceB = -1.0;
	data.Hit = false;
	data.Outside = false;
	data.Forward = false;
	data.HitPlanet = false;

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

	data.Hit = true;
	data.Forward = true;

	if (vpcMag > r)
	{
		// Outside - Forward
		data.Outside = true;
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

IntersectionData GetSphereIntersections(vec3 p, vec3 d, vec3 c, float r)
{
	IntersectionData data;
	data.DistanceA = -1.0;
	data.DistanceB = -1.0;
	data.Hit = false;
	data.Outside = false;
	data.Forward = false;
	data.HitPlanet = false;

	vec3 vpc = c - p;
	float vpcMag = length(vpc);

	// Sphere center is behind p.
	if (dot(vpc, d) < 0.0)
	{
		IntersectionData behindIntersection = TestIntersectionBehind(p, d, c, r);
		IntersectionData planetBehindIntersection = TestIntersectionBehind(p, d, c, u_PlanetRadius);

		if (planetBehindIntersection.Hit && !planetBehindIntersection.Outside)
			return data;

		return behindIntersection;
	}
	// Sphere center projects onto line (p is behind center).
	else
	{
		IntersectionData forwardIntersection = TestIntersectionForward(p, d, c, r);
		IntersectionData planetForwardIntersection = TestIntersectionForward(p, d, c, u_PlanetRadius);
		
		bool planetHit = planetForwardIntersection.Hit;
		bool insidePlanetHit = !forwardIntersection.Outside;

		// Viewer is inside the planet.
		if (planetHit && !planetForwardIntersection.Outside)
			return data;

		// If we didn't hit the planet, take the normal intersections.  Otherwise
		// check if we hit the planet from inside the atmosphere.  If we did, 
		// distanceA is 0.0 (the camera) and distanceB is the intersection point on the planet.
		// If we hit the planet from outside the atmosphere, distanceA is the atmosphere hit,
		// distanceB is the planet hit.
		vec2 forwardDistances = !planetHit
					? vec2(forwardIntersection.DistanceA, forwardIntersection.DistanceB)
					: insidePlanetHit
						? vec2(0.0, planetForwardIntersection.DistanceA)
						: vec2(forwardIntersection.DistanceA, planetForwardIntersection.DistanceA);

		data.HitPlanet = planetHit;
		data.DistanceA = forwardDistances.x;
		data.DistanceB = forwardDistances.y;
		data.Hit = true;
		data.Outside = forwardIntersection.Outside;
		data.Forward = true;

		return data;
	}
}

float InverseLerp(float v, float min, float max)
{
	return (v - min) / (max - min);
}


float Remap(float v, float minOld, float maxOld, float minNew, float maxNew)
{
	return minNew + (v - minOld) * (maxNew - minNew) / (maxOld - minOld);
}

float CalculateLinearDepth(float depth)
{
	float near = u_NearClip;
	float far = u_FarClip;

	float ndcDepth = depth * 2.0 - 1.0;
	float linearDepth = (2.0 * near * far) / (far + near - ndcDepth * (far - near));

	return linearDepth;
}

float ViewerDistanceAttenationFactor(float power)
{
	float attenuationDistanceBegin = u_AtmosphereRadius + u_AtmosphereRadius * u_ViewerAttenuationFactor;
	vec3 planetViewerOffset = u_PlanetPosition - u_CameraPosition;
	float planetViewerSqrMag = dot(planetViewerOffset, planetViewerOffset);
	float sqrAttenuationRadius = attenuationDistanceBegin * attenuationDistanceBegin;
	float isDistanceAttenuated = step(sqrAttenuationRadius, planetViewerSqrMag);

	float attenuationPercent = (1.0 * (1.0 - isDistanceAttenuated)) + isDistanceAttenuated * (1.0 / pow(max(1.0, planetViewerSqrMag / sqrAttenuationRadius), power));
	return attenuationPercent;
}

float SampleCloudDensity(vec3 rayPosition)
{
	float attenuationPercent = ViewerDistanceAttenationFactor(1.0);

	float centerDistance = length(rayPosition - u_PlanetPosition);
	float cloudPercent = clamp(Remap(centerDistance, u_PlanetRadius + u_PlanetRadius * u_LowerCloudOffsetPct, u_AtmosphereRadius - u_AtmosphereRadius * u_UpperCloudOffsetPct, 0.0, 1.0), 0.0, 1.0);
	float distanceDensityMapping = cos(cloudPercent * PI - PI / 2.0);

	vec3 containerSize = vec3(u_AtmosphereRadius * 2.0 * PI);
	vec3 uvw = (containerSize * 0.5 + rayPosition) * (1.0 / 1000.0) * u_CloudScale;

	float time = u_TimeScale * u_ElapsedTime;
	float animationSpeed = u_Animate ? u_AnimationSpeed : 0.0;
	vec3 animationOffset = vec3(time, time * 0.1, time * 0.2) * animationSpeed;
	vec3 shapeSamplePosition = uvw + animationOffset;

	vec4 shape = texture(u_ShapeTexture, shapeSamplePosition) * distanceDensityMapping * attenuationPercent;
	vec4 normalizedShapeWeights = u_ShapeNoiseWeights / dot(u_ShapeNoiseWeights, vec4(1.0));
	float shapeFBM = dot(shape, normalizedShapeWeights);
	float baseShapeDensity = max(0.0, shapeFBM - u_DensityThreshold);

	return min(1.0, baseShapeDensity * u_DensityMultiplier);
}

float Beers(float d)
{
	return exp(-d);
}

float Powder(float d)
{
	return 1.0 - exp(-d * 2.0);
}

float HG(float a, float g)
{
	float g2 = g * g;
	return (1 - g2) / (4.0 * 3.1415 * pow(1.0 + g2 - 2.0 * g * a, 1.5));
}

void main()
{
	vec3 sceneColor = texture(u_SceneTexture, v_TexCoord).rgb;

	vec3 rayOrigin = u_CameraPosition;
	vec3 rayDirection = normalize(v_ViewDirection);

	vec3 atmosphereColor = vec3(0.0, 0.0, 1.0);
	float totalAtmosphericDensity = 0.0;

	float nonLinearDepth = texture(u_DepthTexture, v_TexCoord).r;
	float linearDepth = CalculateLinearDepth(nonLinearDepth);

	IntersectionData cloudMarchData = GetSphereIntersections(rayOrigin, rayDirection, u_PlanetPosition, u_AtmosphereRadius);
	float distanceLimit = max(0.0, min(linearDepth - cloudMarchData.DistanceA, cloudMarchData.DistanceB - cloudMarchData.DistanceA));

	float distanceTravelled = 0.0;
	float stepSize = distanceLimit / float(u_DensitySteps);
	vec3 intersectionPoint = rayOrigin + rayDirection * cloudMarchData.DistanceA;

	vec3 directionToLight = normalize(u_Light.Position - u_CameraPosition);
	float cosLightViewer = dot(rayDirection, directionToLight);
	float hgPhaseFn = HG(cosLightViewer, 0.8f);

	float transmittance = 1.0;
	float lightEnergy = 0.0;

	while (distanceTravelled < distanceLimit)
	{
		rayOrigin = intersectionPoint + rayDirection * distanceTravelled;
		float distanceToPlanetCenter = length(rayOrigin - u_PlanetPosition);

		float height = InverseLerp(distanceToPlanetCenter, u_PlanetRadius, u_AtmosphereRadius);
		float atmosphericDensity = mix(0.00, 0.1, height) * stepSize;
		totalAtmosphericDensity += atmosphericDensity;

		float cloudDensity = SampleCloudDensity(rayOrigin) * stepSize;

		if (cloudDensity > 0.0)
		{
			// Energy reaching viewer = Scattering * Silver Lining (HG Phase) * Powder (from Guerilla games) * multiplier
			float lightTransmittance = Beers(cloudDensity) * hgPhaseFn * Powder(cloudDensity) * u_LuminanceMultiplier;

			// Accumulate energy for each sample.
			lightEnergy += lightTransmittance;

			// This energy is getting abosrbed on it's way out of the cloud.
			transmittance *= Beers(cloudDensity);

			if (transmittance < 0.01)
				break;
		}

		distanceTravelled += stepSize;
	}

	atmosphereColor *= totalAtmosphericDensity * u_AtmosphereStrength;
	vec3 cloudColor = lightEnergy * u_Light.Color;

	float focusedEye = pow(clamp(dot(rayDirection, directionToLight), 0.0, 1.0), .8);
	float sunValue = clamp(HG(focusedEye, 0.9995), 0.0, 1.0) * transmittance;

	vec3 result = sceneColor + cloudColor + atmosphereColor;
	result = vec3(result * (1.0 - sunValue) + u_Light.Color * sunValue);

	result = ACESTonemap(result);
	result = GammaCorrect(result, 2.2);

	o_Color = vec4(result, 1.0);
}