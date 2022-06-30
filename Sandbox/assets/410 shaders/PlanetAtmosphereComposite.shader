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

const float Epsilon = 0.0001;

uniform sampler2D u_SceneTexture;
uniform sampler2D u_DepthTexture;

uniform vec3 u_CloudTintColor;
uniform vec3 u_AtmosphereColor;

uniform float u_NearClip;
uniform float u_FarClip;

uniform float u_ElapsedTime;
uniform float u_TimeScale;
uniform float u_AnimationSpeed;
uniform bool u_Animate;

uniform Light u_Light;

uniform int u_DensitySteps;
uniform int u_LightSteps;
uniform int u_AtmosphericOpticalDepthPoints;


uniform sampler2D u_WeatherMap;
uniform sampler2D u_CurlNoise;
uniform sampler3D u_ShapeTexture;
uniform sampler3D u_DetailTexture;

uniform float u_LowerCloudOffsetPct;
uniform float u_UpperCloudOffsetPct;
uniform float u_ViewerAttenuationFactor;
uniform float u_CloudScale;
uniform float u_DensityMultiplier;
uniform float u_DensityThreshold;


uniform float u_LuminanceMultiplier;
uniform float u_ForwardScatteringConstant;
uniform float u_ExtinctionFactor;
uniform float u_DetailHeightModifier;
uniform float u_TypeWeightMultiplier;
uniform float u_SilverLiningConstant;
uniform float u_PowderConstant;

uniform vec4 u_ShapeNoiseWeights;
uniform vec3 u_DetailNoiseWeights;
uniform vec3 u_CloudTypeWeights;
uniform vec4 u_PhaseParams;

uniform float u_SunSizeAttenuation;
uniform float u_AtmosphereStrength;
uniform float u_AtmosphereDensityFalloff;
uniform vec3 u_AtmosphereScatteringCoefficient;

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

		// HIT PLANET LOGIC
		// If we didn't hit the planet, take the normal intersections.  Otherwise
		// check if we hit the planet from inside the atmosphere.  If we did, 
		// distanceA is 0.0 (the camera) and distanceB is the intersection point on the planet.
		// If we hit the planet from outside the atmosphere, distanceA is the atmosphere hit,
		// distanceB is the planet hit.

		//vec2 forwardDistances = !planetHit
		//			? vec2(forwardIntersection.DistanceA, forwardIntersection.DistanceB)
		//			: insidePlanetHit
		//				? vec2(0.0, planetForwardIntersection.DistanceA)
		//				: vec2(forwardIntersection.DistanceA, planetForwardIntersection.DistanceA);
		// HIT PLANET LOGIC

		vec2 forwardDistances = vec2(forwardIntersection.DistanceA, forwardIntersection.DistanceB);

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
	return minNew + (((v - minOld) / (maxOld - minOld)) * (maxNew - minNew));
}

float CalculateLinearDepth(float depth)
{
	float near = u_NearClip;
	float far = u_FarClip;

	float ndcDepth = depth * 2.0 - 1.0;
	float linearDepth = (2.0 * near * far) / (far + near - ndcDepth * (far - near));

	return linearDepth;
}

float ViewerDistanceAttenuationFactor(float power)
{
	float attenuationDistanceBegin = u_AtmosphereRadius + u_AtmosphereRadius * u_ViewerAttenuationFactor;
	vec3 planetViewerOffset = u_PlanetPosition - u_CameraPosition;
	float planetViewerSqrMag = dot(planetViewerOffset, planetViewerOffset);
	float sqrAttenuationRadius = attenuationDistanceBegin * attenuationDistanceBegin;
	float isDistanceAttenuated = step(sqrAttenuationRadius, planetViewerSqrMag);

	float attenuationPercent = (1.0 * (1.0 - isDistanceAttenuated)) + isDistanceAttenuated * (1.0 / pow(max(1.0, planetViewerSqrMag / sqrAttenuationRadius), power));
	return attenuationPercent;
}

mat3 RotateX(float theta)
{
	float c = cos(theta);
	float s = sin(theta);
	return mat3(
		vec3(1, 0, 0),
		vec3(0, c, -s),
		vec3(0, s, c)
	);
}

mat3 RotateY(float theta)
{
	float c = cos(theta);
	float s = sin(theta);
	return mat3(
		vec3(c, 0, s),
		vec3(0, 1, 0),
		vec3(-s, 0, c)
	);
}

mat3 RotateZ(float theta)
{
	float c = cos(theta);
	float s = sin(theta);
	return mat3(
		vec3(c, -s, 0),
		vec3(s, c, 0),
		vec3(0, 0, 1)
	);
}

float CubicPulse(float t, float c, float w)
{
	t = abs(t - c);
	if (t > w) return 0.0;
	t /= w;
	return 1.0 - t * t * (3.0 - 2.0 * t);
}

float Stratus(float t)
{
	float c = 0.125;
	float w = 0.040;
	return CubicPulse(t, c, w);
}

float Cumulus(float t)
{
	float c = 0.265;
	float w = 0.200;
	return CubicPulse(t, c, w);
}

float Cumulonimbus(float t)
{
	float c = .55;
	float w = .52;
	return CubicPulse(t, c, w);
}

float GetHeightFractionForPoint(float distanceToCenter, vec2 cloudMinMax)
{
	return clamp(((distanceToCenter - cloudMinMax.x) / (cloudMinMax.y - cloudMinMax.x)), 0.0, 1.0);
}

float GetHeightGradientFromPoint(float distanceToCenter, vec2 cloudMinMax)
{
	float heightFraction = GetHeightFractionForPoint(distanceToCenter, cloudMinMax);
	vec3 heightMapping = vec3(Stratus(heightFraction), Cumulus(heightFraction), Cumulonimbus(heightFraction));
	vec3 normalizedCloudTypeWeights = u_CloudTypeWeights / dot(u_CloudTypeWeights, vec3(1.0));
	float heightValue = dot(heightMapping, normalizedCloudTypeWeights);

	return heightValue * u_TypeWeightMultiplier;
}

float Beers(float d)
{
	return exp(-d);
}

float HG(float a, float g)
{
	float g2 = g * g;
	return (1 - g2) / (4.0 * 3.1415 * pow(1.0 + g2 - 2.0 * g * a, 1.5));
}

float RayleighPhaseFunction(float a)
{
	float k = 3.0 / (16.0 * PI);
	return k * (1.0 + a * a);
}

float MiePhaseFunction(float g, float a)
{
	float k = 3.0 / (8.0 * PI * 1.0) * (1.0 - g * g) / (2.0 + g * g);
	return k * (1.0 + a * a) / pow(1.0 + g * g - 2.0 * g * a, 1.5);
}

float PhaseFn(float angle)
{
	float blend = 0.5;
	float hgBlend = HG(angle, u_PhaseParams.x) * (1.0 - blend) + HG(angle, u_PhaseParams.y) * blend;
	return u_PhaseParams.z + hgBlend * u_PhaseParams.w;
}

float CalculateAtmosphericDensity(vec3 p, vec3 c, float rNear, float rFar)
{
	float distanceToCenter = length(p - c) - rNear;
	float height = distanceToCenter / (rFar - rNear);
	float atmosphericDensity = exp(-height * u_AtmosphereDensityFalloff) * (1.0 - height);

	return atmosphericDensity;
}

float CalculateOpticalDepth(vec3 rayOrigin, vec3 rayDirection, float rayLength)
{
	vec3 scatterPoint = rayOrigin;
	float stepSize = rayLength / float(u_AtmosphericOpticalDepthPoints);
	vec3 inScatteredLight = vec3(0.0);

	float opticalDepth = 0.0;

	for (int i = 0; i < u_AtmosphericOpticalDepthPoints; i++)
	{
		float density = CalculateAtmosphericDensity(scatterPoint, u_PlanetPosition, u_PlanetRadius, u_AtmosphereRadius);
		opticalDepth += density * stepSize;
		scatterPoint += rayDirection * stepSize;
	}

	return opticalDepth;
}

vec3 March(vec3 rayOrigin, vec3 rayDirection, vec3 directionToLight)
{
	float nonLinearDepth = texture(u_DepthTexture, v_TexCoord).r;
	float linearDepth = CalculateLinearDepth(nonLinearDepth);

	IntersectionData intersectionData = GetSphereIntersections(rayOrigin, rayDirection, u_PlanetPosition, u_AtmosphereRadius);
	float marchDistance = intersectionData.DistanceB - intersectionData.DistanceA;
	float distanceToAtmosphere = intersectionData.Hit && intersectionData.HitPlanet ? length(rayOrigin + rayDirection * intersectionData.DistanceA - rayOrigin) : 0.0;
	float distanceLimit = min(linearDepth - distanceToAtmosphere, marchDistance);

	float distanceTraveled = 0.0;
	float stepSize = distanceLimit / float(u_DensitySteps);
	vec3 intersectionPoint = rayOrigin + rayDirection * intersectionData.DistanceA;

	vec3 inScatteredLight = vec3(0.0);

	while (distanceTraveled < distanceLimit)
	{
		rayOrigin = intersectionPoint + rayDirection * distanceTraveled;
		float atmosphereDensity = CalculateAtmosphericDensity(rayOrigin, u_PlanetPosition, u_PlanetRadius, u_AtmosphereRadius);

		vec3 directionToSun = normalize(u_Light.Position - rayOrigin);
		IntersectionData sunSphereIntersectData = GetSphereIntersections(rayOrigin, directionToSun, u_PlanetPosition, u_AtmosphereRadius);

		float rayleigh = RayleighPhaseFunction(max(0.0, dot(rayDirection, directionToSun)));

		float distanceToSun = max(0.0, sunSphereIntersectData.DistanceB - sunSphereIntersectData.DistanceA);
		float sunRayOpticalDepth = CalculateOpticalDepth(rayOrigin, directionToSun, distanceToSun);
		float viewRayOpticalDepth = CalculateOpticalDepth(rayOrigin, -rayDirection, distanceTraveled) * rayleigh;

		vec3 atmosphereTransmittance = exp(-(sunRayOpticalDepth + viewRayOpticalDepth) * u_AtmosphereScatteringCoefficient);

		inScatteredLight += atmosphereTransmittance * atmosphereDensity * u_AtmosphereScatteringCoefficient;
		distanceTraveled += stepSize;
	}

	vec3 atmosphereColor = inScatteredLight * u_AtmosphereStrength * stepSize / u_PlanetRadius;
	return atmosphereColor;
}

void main()
{
	vec3 sceneColor = texture(u_SceneTexture, v_TexCoord).rgb;
	vec3 rayOrigin = u_CameraPosition;
	vec3 rayDirection = normalize(v_ViewDirection);

	vec3 directionToLight = normalize(u_Light.Position - u_CameraPosition);

	vec3 atmosphereColor = March(rayOrigin, rayDirection, directionToLight);

	float focusedEye = pow(max(0.0, dot(rayDirection, directionToLight)), u_SunSizeAttenuation);
	float sunValue = clamp(HG(focusedEye, 0.9999), 0.0, 1.0);

	vec3 result = sceneColor + atmosphereColor;
	result = vec3(result * (1.0 - sunValue) + u_Light.Color * sunValue);

	result = ACESTonemap(result);
	result = GammaCorrect(result, 2.2);

	o_Color = vec4(result, 1.0);
}