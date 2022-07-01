#type compute
#version 450
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout(rgba32f, binding = 0) restrict writeonly uniform imageCube o_Result;

float PI = 3.141592653589793;
float RAD_TO_DEG = 180.0 / PI;
float PLANET_RADIUS = 6378000.0;
float SAFETY_ALTITUDE = 50.0;
float SUN_RADIUS = 0.004654793;
float DIST_TO_EDGE = 1571524.413613;
float ATMOSPHERE_WIDTH = 100000.0;
float SUN_RAD_START = 280;
double SPECTRUM_WAVELENGTHS[55] = { 300.0,  340.0,  380.0,  420.0,  460.0,  500.0,  540.0,  580.0,
											620.0,  660.0,  700.0,  740.0,  780.0,  820.0,  860.0,  900.0,
											940.0,  980.0,  1020.0, 1060.0, 1100.0, 1140.0, 1180.0, 1220.0,
											1260.0, 1300.0, 1340.0, 1380.0, 1420.0, 1460.0, 1500.0, 1540.0,
											1580.0, 1620.0, 1660.0, 1700.0, 1740.0, 1780.0, 1820.0, 1860.0,
											1900.0, 1940.0, 1980.0, 2020.0, 2060.0, 2100.0, 2140.0, 2180.0,
											2220.0, 2260.0, 2300.0, 2340.0, 2380.0, 2420.0, 2460.0 };

int SPECTRAL_RESPONSE_COUNT = 95;
// Spectral response table used for converting spectrum to XYZ.
const vec3 SPECTRAL_RESPONSE[] = {
	vec3(0.000129900000f, 0.000003917000f, 0.000606100000f),
	vec3(0.000232100000f, 0.000006965000f, 0.001086000000f),
	vec3(0.000414900000f, 0.000012390000f, 0.001946000000f),
	vec3(0.000741600000f, 0.000022020000f, 0.003486000000f),
	vec3(0.001368000000f, 0.000039000000f, 0.006450001000f),
	vec3(0.002236000000f, 0.000064000000f, 0.010549990000f),
	vec3(0.004243000000f, 0.000120000000f, 0.020050010000f),
	vec3(0.007650000000f, 0.000217000000f, 0.036210000000f),
	vec3(0.014310000000f, 0.000396000000f, 0.067850010000f),
	vec3(0.023190000000f, 0.000640000000f, 0.110200000000f),
	vec3(0.043510000000f, 0.001210000000f, 0.207400000000f),
	vec3(0.077630000000f, 0.002180000000f, 0.371300000000f),
	vec3(0.134380000000f, 0.004000000000f, 0.645600000000f),
	vec3(0.214770000000f, 0.007300000000f, 1.039050100000f),
	vec3(0.283900000000f, 0.011600000000f, 1.385600000000f),
	vec3(0.328500000000f, 0.016840000000f, 1.622960000000f),
	vec3(0.348280000000f, 0.023000000000f, 1.747060000000f),
	vec3(0.348060000000f, 0.029800000000f, 1.782600000000f),
	vec3(0.336200000000f, 0.038000000000f, 1.772110000000f),
	vec3(0.318700000000f, 0.048000000000f, 1.744100000000f),
	vec3(0.290800000000f, 0.060000000000f, 1.669200000000f),
	vec3(0.251100000000f, 0.073900000000f, 1.528100000000f),
	vec3(0.195360000000f, 0.090980000000f, 1.287640000000f),
	vec3(0.142100000000f, 0.112600000000f, 1.041900000000f),
	vec3(0.095640000000f, 0.139020000000f, 0.812950100000f),
	vec3(0.057950010000f, 0.169300000000f, 0.616200000000f),
	vec3(0.032010000000f, 0.208020000000f, 0.465180000000f),
	vec3(0.014700000000f, 0.258600000000f, 0.353300000000f),
	vec3(0.004900000000f, 0.323000000000f, 0.272000000000f),
	vec3(0.002400000000f, 0.407300000000f, 0.212300000000f),
	vec3(0.009300000000f, 0.503000000000f, 0.158200000000f),
	vec3(0.029100000000f, 0.608200000000f, 0.111700000000f),
	vec3(0.063270000000f, 0.710000000000f, 0.078249990000f),
	vec3(0.109600000000f, 0.793200000000f, 0.057250010000f),
	vec3(0.165500000000f, 0.862000000000f, 0.042160000000f),
	vec3(0.225749900000f, 0.914850100000f, 0.029840000000f),
	vec3(0.290400000000f, 0.954000000000f, 0.020300000000f),
	vec3(0.359700000000f, 0.980300000000f, 0.013400000000f),
	vec3(0.433449900000f, 0.994950100000f, 0.008749999000f),
	vec3(0.512050100000f, 1.000000000000f, 0.005749999000f),
	vec3(0.594500000000f, 0.995000000000f, 0.003900000000f),
	vec3(0.678400000000f, 0.978600000000f, 0.002749999000f),
	vec3(0.762100000000f, 0.952000000000f, 0.002100000000f),
	vec3(0.842500000000f, 0.915400000000f, 0.001800000000f),
	vec3(0.916300000000f, 0.870000000000f, 0.001650001000f),
	vec3(0.978600000000f, 0.816300000000f, 0.001400000000f),
	vec3(1.026300000000f, 0.757000000000f, 0.001100000000f),
	vec3(1.056700000000f, 0.694900000000f, 0.001000000000f),
	vec3(1.062200000000f, 0.631000000000f, 0.000800000000f),
	vec3(1.045600000000f, 0.566800000000f, 0.000600000000f),
	vec3(1.002600000000f, 0.503000000000f, 0.000340000000f),
	vec3(0.938400000000f, 0.441200000000f, 0.000240000000f),
	vec3(0.854449900000f, 0.381000000000f, 0.000190000000f),
	vec3(0.751400000000f, 0.321000000000f, 0.000100000000f),
	vec3(0.642400000000f, 0.265000000000f, 0.000049999990f),
	vec3(0.541900000000f, 0.217000000000f, 0.000030000000f),
	vec3(0.447900000000f, 0.175000000000f, 0.000020000000f),
	vec3(0.360800000000f, 0.138200000000f, 0.000010000000f),
	vec3(0.283500000000f, 0.107000000000f, 0.000000000000f),
	vec3(0.218700000000f, 0.081600000000f, 0.000000000000f),
	vec3(0.164900000000f, 0.061000000000f, 0.000000000000f),
	vec3(0.121200000000f, 0.044580000000f, 0.000000000000f),
	vec3(0.087400000000f, 0.032000000000f, 0.000000000000f),
	vec3(0.063600000000f, 0.023200000000f, 0.000000000000f),
	vec3(0.046770000000f, 0.017000000000f, 0.000000000000f),
	vec3(0.032900000000f, 0.011920000000f, 0.000000000000f),
	vec3(0.022700000000f, 0.008210000000f, 0.000000000000f),
	vec3(0.015840000000f, 0.005723000000f, 0.000000000000f),
	vec3(0.011359160000f, 0.004102000000f, 0.000000000000f),
	vec3(0.008110916000f, 0.002929000000f, 0.000000000000f),
	vec3(0.005790346000f, 0.002091000000f, 0.000000000000f),
	vec3(0.004109457000f, 0.001484000000f, 0.000000000000f),
	vec3(0.002899327000f, 0.001047000000f, 0.000000000000f),
	vec3(0.002049190000f, 0.000740000000f, 0.000000000000f),
	vec3(0.001439971000f, 0.000520000000f, 0.000000000000f),
	vec3(0.000999949300f, 0.000361100000f, 0.000000000000f),
	vec3(0.000690078600f, 0.000249200000f, 0.000000000000f),
	vec3(0.000476021300f, 0.000171900000f, 0.000000000000f),
	vec3(0.000332301100f, 0.000120000000f, 0.000000000000f),
	vec3(0.000234826100f, 0.000084800000f, 0.000000000000f),
	vec3(0.000166150500f, 0.000060000000f, 0.000000000000f),
	vec3(0.000117413000f, 0.000042400000f, 0.000000000000f),
	vec3(0.000083075270f, 0.000030000000f, 0.000000000000f),
	vec3(0.000058706520f, 0.000021200000f, 0.000000000000f),
	vec3(0.000041509940f, 0.000014990000f, 0.000000000000f),
	vec3(0.000029353260f, 0.000010600000f, 0.000000000000f),
	vec3(0.000020673830f, 0.000007465700f, 0.000000000000f),
	vec3(0.000014559770f, 0.000005257800f, 0.000000000000f),
	vec3(0.000010253980f, 0.000003702900f, 0.000000000000f),
	vec3(0.000007221456f, 0.000002607800f, 0.000000000000f),
	vec3(0.000005085868f, 0.000001836600f, 0.000000000000f),
	vec3(0.000003581652f, 0.000001293400f, 0.000000000000f),
	vec3(0.000002522525f, 0.000000910930f, 0.000000000000f),
	vec3(0.000001776509f, 0.000000641530f, 0.000000000000f),
	vec3(0.000001251141f, 0.000000451810f, 0.000000000000f)
};
double SPECTRAL_RESPONSE_START = 360.0;
double SPECTRAL_RESPONSE_STEP = 5.0;
int    SPECTRUM_CHANNELS = 55;
double SPECTRUM_STEP = 40;

uniform float u_Altitude;
uniform float u_Elevation;
uniform float u_Azimuth;
uniform float u_Visibility;
uniform float u_Albedo;
uniform float u_Exposure;

uniform int u_TotalCoefsSingleConfig;
uniform int u_TotalCoefsAllConfigs;
uniform int u_Rank;

uniform int    u_Channels;
uniform float u_ChannelStart;
uniform float u_ChannelWidth;

uniform int u_ApplyTonemap;


layout(std430, binding = 1) buffer RadianceData
{
	float radData[];
};

uniform int sunOffset;
uniform int sunStride;
layout(std430, binding = 2) buffer SunMetaData
{
	double sunBreaks[];
};

uniform int zenithOffset;
uniform int zenithStride;
layout(std430, binding = 3) buffer ZenithMetaData
{
	double zenithBreaks[];
};

uniform int emphOffset;
layout(std430, binding = 4) buffer EmphMetaData
{
	double emphBreaks[];
};

layout(std430, binding = 5) buffer VisibilitiesData
{
	double visibilitiesRad[];
};

layout(std430, binding = 6) buffer AlbedosData
{
	double albedosRad[];
};

layout(std430, binding = 7) buffer AltitudesData
{
	double altitudesRad[];
};

layout(std430, binding = 8) buffer ElevationsData
{
	double elevationsRad[];
};


struct Parameters
{
	float theta;
	float gamma;
	float shadow;
	float zero;
	float elevation;
	float altitude;
	float visibility;
	float albedo;
};

struct InterpolationParameter
{
	double factor;
	int    index;
};

struct AngleParameters
{
	InterpolationParameter gamma, alpha, zero;
};

struct ControlParameters
{
	/// 16 sets of parameters that will be bi-linearly interpolated
	int coefficientOffsets[16];
	double interpolationFactors[4];
};


Parameters ComputeParameters(
	vec3 viewpoint,
	vec3 viewDirection,
	float   groundLevelSolarElevationAtOrigin,
	float   groundLevelSolarAzimuthAtOrigin,
	float   visibility,
	float   albedo)
{
	//assert(viewpoint.z >= 0.0);
	//assert(magnitude(viewDirection) > 0.0);
	//assert(visibility >= 0.0);
	//assert(albedo >= 0.0 && albedo <= 1.0);

	Parameters params;
	params.visibility = visibility;
	params.albedo = albedo;

	// Shift viewpoint about safety altitude up

	vec3 centerOfTheEarth = vec3(0.0, -PLANET_RADIUS, 0.0);
	vec3 toViewpoint = viewpoint - centerOfTheEarth;
	vec3 toViewpointN = normalize(toViewpoint);

	float distanceToView = length(toViewpoint) + SAFETY_ALTITUDE;
	vec3 toShiftedViewpoint = toViewpointN * distanceToView;
	vec3 shiftedViewpoint = centerOfTheEarth + toShiftedViewpoint;

	vec3 viewDirectionN = normalize(viewDirection);

	// Compute altitude of viewpoint

	params.altitude = distanceToView - PLANET_RADIUS;
	params.altitude = max(params.altitude, 0.0);

	// Direction to sun

	vec3 directionToSunN;
	directionToSunN.x = cos(groundLevelSolarAzimuthAtOrigin) * cos(groundLevelSolarElevationAtOrigin);
	directionToSunN.y = sin(groundLevelSolarElevationAtOrigin);
	directionToSunN.z = sin(groundLevelSolarAzimuthAtOrigin) * cos(groundLevelSolarElevationAtOrigin);

	// Solar elevation at viewpoint (more precisely, solar elevation at the point
	// on the ground directly below viewpoint)

	float dotZenithSun = dot(toViewpointN, directionToSunN);
	params.elevation = 0.5 * PI - acos(dotZenithSun);

	// Altitude-corrected view direction

	vec3 correctViewN;
	if (distanceToView > PLANET_RADIUS)
	{
		vec3 lookAt = shiftedViewpoint + viewDirectionN;

		float correction = sqrt(distanceToView * distanceToView - PLANET_RADIUS * PLANET_RADIUS) / distanceToView;

		vec3 toNewOrigin = toViewpointN * (distanceToView - correction);
		vec3 newOrigin = centerOfTheEarth + toNewOrigin;
		vec3 correctView = lookAt - newOrigin;

		correctViewN = normalize(correctView);
	}
	else
	{
		correctViewN = viewDirectionN;
	}

	// Sun angle (gamma) - no correction

	float dotProductSun = dot(viewDirectionN, directionToSunN);
	params.gamma = acos(dotProductSun);

	// Shadow angle - requires correction

	float effectiveElevation = groundLevelSolarElevationAtOrigin;
	float effectiveAzimuth = groundLevelSolarAzimuthAtOrigin;
	float shadowAngle = effectiveElevation + PI * 0.5;

	vec3 shadowDirectionN = vec3(cos(shadowAngle) * cos(effectiveAzimuth), sin(shadowAngle), cos(shadowAngle) * sin(effectiveAzimuth));

	float dotProductShadow = dot(correctViewN, shadowDirectionN);
	params.shadow = acos(dotProductShadow);

	// Zenith angle (theta) - corrected version stored in otherwise unused zero
	// angle

	float cosThetaCor = dot(correctViewN, toViewpointN);
	params.zero = acos(cosThetaCor);

	// Zenith angle (theta) - uncorrected version goes outside

	float cosTheta = dot(viewDirectionN, toViewpointN);
	params.theta = acos(cosTheta);

	return params;
}


double radiansToDegrees(const double radians)
{
	return radians * RAD_TO_DEG;
}

vec3 GetCubeMapCoords()
{
	vec2 st = (gl_GlobalInvocationID.xy) / vec2(imageSize(o_Result) - 1.0);
	vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

	vec3 coords;

	if (gl_GlobalInvocationID.z == 0)	coords = vec3(1.0, uv.y, -uv.x);
	else if (gl_GlobalInvocationID.z == 1)	coords = vec3(-1.0, uv.y, uv.x);
	else if (gl_GlobalInvocationID.z == 2)	coords = vec3(uv.x, 1.0, -uv.y);
	else if (gl_GlobalInvocationID.z == 3)	coords = vec3(uv.x, -1.0, uv.y);
	else if (gl_GlobalInvocationID.z == 4)	coords = vec3(uv.x, uv.y, 1.0);
	else if (gl_GlobalInvocationID.z == 5)	coords = vec3(-uv.x, uv.y, -1.0);

	return normalize(coords);
}

InterpolationParameter InterpolateSunBreaks(double queryVal)
{
	const double clamped = clamp(queryVal, sunBreaks[0], sunBreaks[sunBreaks.length() - 1]);

	int nextIndex;
	for (nextIndex = 1; nextIndex < sunBreaks.length(); nextIndex++)
	{
		if (sunBreaks[nextIndex] > clamped)
			break;
	}

	InterpolationParameter parameter;
	parameter.index = nextIndex - 1;
	if (parameter.index >= sunBreaks.length())
	{
		parameter.factor = 0.0;
	}
	else
	{
		double previousValue = sunBreaks[nextIndex - 1];
		double nextValue = sunBreaks[nextIndex];
		parameter.factor = (clamped - previousValue) / (nextValue - previousValue);
	}

	return parameter;
}
InterpolationParameter InterpolateZenithBreaks(double queryVal)
{
	const double clamped = clamp(queryVal, zenithBreaks[0], zenithBreaks[zenithBreaks.length() - 1]);

	int nextIndex;
	for (nextIndex = 1; nextIndex < zenithBreaks.length(); nextIndex++)
	{
		if (zenithBreaks[nextIndex] > clamped)
			break;
	}

	InterpolationParameter parameter;
	parameter.index = nextIndex - 1;
	if (parameter.index >= zenithBreaks.length())
	{
		parameter.factor = 0.0;
	}
	else
	{
		double previousValue = zenithBreaks[nextIndex - 1];
		double nextValue = zenithBreaks[nextIndex];
		parameter.factor = (clamped - previousValue) / (nextValue - previousValue);
	}

	return parameter;
}
InterpolationParameter InterpolateEmphBreaks(double queryVal)
{
	const double clamped = clamp(queryVal, emphBreaks[0], emphBreaks[emphBreaks.length() - 1]);

	int nextIndex;
	for (nextIndex = 1; nextIndex < emphBreaks.length(); nextIndex++)
	{
		if (emphBreaks[nextIndex] > clamped)
			break;
	}

	InterpolationParameter parameter;
	parameter.index = nextIndex - 1;
	if (parameter.index >= emphBreaks.length())
	{
		parameter.factor = 0.0;
	}
	else
	{
		double previousValue = emphBreaks[nextIndex - 1];
		double nextValue = emphBreaks[nextIndex];
		parameter.factor = (clamped - previousValue) / (nextValue - previousValue);
	}

	return parameter;
}
InterpolationParameter InterpolateVisibilities(double queryVal)
{
	const double clamped = clamp(queryVal, visibilitiesRad[0], visibilitiesRad[visibilitiesRad.length() - 1]);
	
	int nextIndex;
	for (nextIndex = 1; nextIndex < visibilitiesRad.length(); nextIndex++)
	{
		if (visibilitiesRad[nextIndex] > clamped)
			break;
	}
	
	InterpolationParameter parameter;
	parameter.index = nextIndex - 1;
	if (parameter.index >= visibilitiesRad.length())
	{
		parameter.factor = 0.0;
	}
	else
	{
		double previousValue = visibilitiesRad[nextIndex - 1];
		double nextValue = visibilitiesRad[nextIndex];
		parameter.factor = (clamped - previousValue) / (nextValue - previousValue);
	}
	
	return parameter;
}
InterpolationParameter InterpolateAlbedos(double queryVal)
{
	const double clamped = clamp(queryVal, albedosRad[0], albedosRad[albedosRad.length() - 1]);

	int nextIndex;
	for (nextIndex = 1; nextIndex < albedosRad.length(); nextIndex++)
	{
		if (albedosRad[nextIndex] > clamped)
			break;
	}

	InterpolationParameter parameter;
	parameter.index = nextIndex - 1;
	if (parameter.index >= albedosRad.length())
	{
		parameter.factor = 0.0;
	}
	else
	{
		double previousValue = albedosRad[nextIndex - 1];
		double nextValue = albedosRad[nextIndex];
		parameter.factor = (clamped - previousValue) / (nextValue - previousValue);
	}

	return parameter;
}
InterpolationParameter InterpolateAltitudes(double queryVal)
{
	const double clamped = clamp(queryVal, altitudesRad[0], altitudesRad[altitudesRad.length() - 1]);

	int nextIndex;
	for (nextIndex = 1; nextIndex < altitudesRad.length(); nextIndex++)
	{
		if (altitudesRad[nextIndex] > clamped)
			break;
	}

	InterpolationParameter parameter;
	parameter.index = nextIndex - 1;
	if (parameter.index >= altitudesRad.length())
	{
		parameter.factor = 0.0;
	}
	else
	{
		double previousValue = altitudesRad[nextIndex - 1];
		double nextValue = altitudesRad[nextIndex];
		parameter.factor = (clamped - previousValue) / (nextValue - previousValue);
	}

	return parameter;
}
InterpolationParameter InterpolateElevations(double queryVal)
{
	const double clamped = clamp(queryVal, elevationsRad[0], elevationsRad[elevationsRad.length() - 1]);

	int nextIndex;
	for (nextIndex = 1; nextIndex < elevationsRad.length(); nextIndex++)
	{
		if (elevationsRad[nextIndex] > clamped)
			break;
	}

	InterpolationParameter parameter;
	parameter.index = nextIndex - 1;
	if (parameter.index >= elevationsRad.length())
	{
		parameter.factor = 0.0;
	}
	else
	{
		double previousValue = elevationsRad[nextIndex - 1];
		double nextValue = elevationsRad[nextIndex];
		parameter.factor = (clamped - previousValue) / (nextValue - previousValue);
	}

	return parameter;
}

/// Evaluates piecewise linear approximation
double evalPL(int offset, const double factor)
{
	return (double(radData[offset + 1]) - double(radData[offset])) * factor + double(radData[offset]);
}

int GetCoefficientSpaceOffset
(
	const int totalCoefsSingleConfig,
	const int elevation,
	const int altitude,
	const int visibility,
	const int albedo,
	const int wavelength
)
{
	return (totalCoefsSingleConfig *
			(wavelength + u_Channels * elevation + u_Channels * elevationsRad.length() * altitude +
				u_Channels * elevationsRad.length() * altitudesRad.length() * albedo +
				u_Channels * elevationsRad.length() * altitudesRad.length() * albedosRad.length() * visibility));
}

double Reconstruct(const AngleParameters radianceParameters, int baseChannelOffset)
{
	// The original image was emphasized (for radiance only), re-parametrized into gamma-alpha space,
	// decomposed into sum of rankRad outer products of 'sun' and 'zenith' vectors and these vectors were
	// stored as piece-wise polynomial approximation. Here the process is reversed (for one point).

	double result = 0.0;
	for (int r = 0; r < u_Rank; ++r)
	{
		// Restore the right value in the 'sun' vector
		int sunBaseOffset = baseChannelOffset + sunOffset + r * sunStride + radianceParameters.gamma.index;
		const double sunParam = evalPL(sunBaseOffset, radianceParameters.gamma.factor);
		barrier();
		memoryBarrier();

		// Restore the right value in the 'zenith' vector
		int zenithBaseOffset = baseChannelOffset + zenithOffset + r * zenithStride + radianceParameters.alpha.index;
		const double zenithParam = evalPL(zenithBaseOffset, radianceParameters.alpha.factor);
		barrier();
		memoryBarrier();

		// Accumulate their "outer" product
		result += sunParam * zenithParam;
	}

	// De-emphasize (for radiance only)
	if (emphBreaks.length() > 0)
	{
		int emphBaseOffset = baseChannelOffset + emphOffset + radianceParameters.zero.index;
		const double emphParam = evalPL(emphBaseOffset, radianceParameters.zero.factor);
		barrier();
		memoryBarrier();

		result *= emphParam;
		result = max(result, 0.0);
	}

	return result;
}

double InterpolateDepth3(const AngleParameters angleParameters, const ControlParameters controlParameters, int offset)
{
	int level = 3;
	double low = Reconstruct(angleParameters, controlParameters.coefficientOffsets[offset]);
	double high = Reconstruct(angleParameters, controlParameters.coefficientOffsets[offset + (1 << (3 - level))]);
	return controlParameters.interpolationFactors[level] < 1e-6 ? low : mix(low, high, controlParameters.interpolationFactors[level]);
}

double InterpolateDepth2(const AngleParameters angleParameters, const ControlParameters controlParameters, int offset)
{
	int level = 2;
	double low = InterpolateDepth3(angleParameters, controlParameters, offset);
	double high = InterpolateDepth3(angleParameters, controlParameters, offset + (1 << (3 - level)));
	return controlParameters.interpolationFactors[level] < 1e-6 ? low : mix(low, high, controlParameters.interpolationFactors[level]);
}

double InterpolateDepth1(const AngleParameters angleParameters, const ControlParameters controlParameters, int offset)
{
	int level = 1;
	double low = InterpolateDepth2(angleParameters, controlParameters, offset);
	double high = InterpolateDepth2(angleParameters, controlParameters, offset + (1 << (3 - level)));
	return controlParameters.interpolationFactors[level] < 1e-6 ? low : mix(low, high, controlParameters.interpolationFactors[level]);
}

double InterpolateDepth0(const AngleParameters angleParameters, const ControlParameters controlParameters)
{
	int level = 0;
	int offset = 0;
	double low = InterpolateDepth1(angleParameters, controlParameters, offset);
	double high = InterpolateDepth1(angleParameters, controlParameters, offset + (1 << (3 - level)));
	return controlParameters.interpolationFactors[level] < 1e-6 ? low : mix(low, high, controlParameters.interpolationFactors[level]);
}

double Interpolate(const AngleParameters angleParameters, const ControlParameters controlParameters)
{
	/// Starts at level 0 and recursively goes down to level 4 while computing offset to the control
	/// parameters array. There it reconstructs radiance. When returning from recursion interpolates
	/// according to elevation, altitude, albedo and visibility at level 3, 2, 1 and 0, respectively.

	return InterpolateDepth0(angleParameters, controlParameters);
}


double EvaluateSkyRadiance(Parameters params, double wavelength)
{
	if (wavelength < u_ChannelStart || wavelength >= (u_ChannelStart + u_Channels * u_ChannelWidth))
		return 0.0;

	const int channelIndex = int(floor((wavelength - u_ChannelStart) / u_ChannelWidth));

	AngleParameters angleParameters;
	angleParameters.gamma = InterpolateSunBreaks(params.gamma);

	if (emphBreaks.length() > 0)
	{ 
		angleParameters.alpha = InterpolateZenithBreaks(params.elevation < 0.0 ? params.shadow : params.zero);
		angleParameters.zero = InterpolateEmphBreaks(params.zero);
	}
	else
	{ // for polarisation
		angleParameters.alpha = InterpolateZenithBreaks(params.theta);
	}
	
	const InterpolationParameter visibilityParam = InterpolateVisibilities(params.visibility);
	const InterpolationParameter albedoParam = InterpolateAlbedos(params.albedo);
	const InterpolationParameter altitudeParam = InterpolateAltitudes(params.altitude);
	const InterpolationParameter elevationParam = InterpolateElevations(radiansToDegrees(params.elevation));
	
	ControlParameters controlParameters;
	for (int i = 0; i < 16; ++i)
	{
		const int visibilityIndex = min(visibilityParam.index + i / 8, int(visibilitiesRad.length() - 1));
		const int albedoIndex =		min(albedoParam.index + (i % 8) / 4, int(albedosRad.length() - 1));
		const int altitudeIndex =	min(altitudeParam.index + (i % 4) / 2, int(altitudesRad.length() - 1));
		const int elevationIndex =	min(elevationParam.index + i % 2, int(elevationsRad.length() - 1));
	
		controlParameters.coefficientOffsets[i] = 
			GetCoefficientSpaceOffset(u_TotalCoefsSingleConfig, elevationIndex, altitudeIndex, visibilityIndex, albedoIndex, channelIndex);
	}
	barrier();
	memoryBarrier();

	controlParameters.interpolationFactors[0] = visibilityParam.factor;
	controlParameters.interpolationFactors[1] = albedoParam.factor;
	controlParameters.interpolationFactors[2] = altitudeParam.factor;
	controlParameters.interpolationFactors[3] = elevationParam.factor;
	
	const double result = Interpolate(angleParameters, controlParameters);
	return max(0.0, result);
}

vec3 SpectrumToRGB(double spectrum[55])
{
	vec3 xyz = vec3(0.0);
	for (int wl = 0; wl < SPECTRUM_CHANNELS; wl++)
	{
		const int responseIdx = int((SPECTRUM_WAVELENGTHS[wl] - SPECTRAL_RESPONSE_START) / SPECTRAL_RESPONSE_STEP);
		if (0 <= responseIdx && responseIdx < SPECTRAL_RESPONSE_COUNT)
			xyz = xyz + SPECTRAL_RESPONSE[responseIdx] * float(spectrum[wl]);
	}
	barrier();
	memoryBarrier();

	xyz = xyz * float(SPECTRUM_STEP);
	// XYZ to sRGB
	vec3 rgb;
	rgb.x = 3.2404542 * xyz.x - 1.5371385 * xyz.y - 0.4985314 * xyz.z;
	rgb.y = -0.9692660 * xyz.x + 1.8760108 * xyz.y + 0.0415560 * xyz.z;
	rgb.z = 0.0556434 * xyz.x - 0.2040259 * xyz.y + 1.0572252 * xyz.z;

	return rgb;
}

vec3 Evaluate()
{
	const vec3 viewDir = GetCubeMapCoords();
	const vec3 viewPoint = vec3(0.0, u_Altitude, 0.0);
	Parameters params = ComputeParameters(viewPoint, viewDir, u_Elevation, u_Azimuth, u_Visibility, u_Albedo);
	barrier();
	memoryBarrier();

	double spectrum[55];
	for (int i = 0; i < 55; i++)
		spectrum[i] = EvaluateSkyRadiance(params, SPECTRUM_WAVELENGTHS[i]);
	barrier();
	memoryBarrier();

	const vec3 rgb = SpectrumToRGB(spectrum);
	return rgb;
}

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

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	vec3 skyRadiance = Evaluate();
	vec3 pixel = skyRadiance;
	if(u_ApplyTonemap == 1)
		pixel = ACESTonemap(pixel);
	float expMult = pow(2.0, u_Exposure);
	pixel = pow(pixel * expMult, vec3(1.0 / 2.2));
	imageStore(o_Result, ivec3(gl_GlobalInvocationID), vec4(pixel, 1.0));
}
