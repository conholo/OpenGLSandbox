#define RADIANCE_SUN_BREAKS					0
#define RADIANCE_ZENITH_BREAKS				1
#define RADIANCE_EMPH_BREAKS				2
#define RADIANCE_VISIBLITIES				3
#define RADIANCE_ALBEDOS					4
#define RADIANCE_ALTITUDES					5
#define RADIANCE_ELEVATIONS					6

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

struct TransmittanceParameters
{
	InterpolationParameter altitude;
	InterpolationParameter distance;
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

double radiansToDegrees(const double radians)
{
	return radians * RAD_TO_DEG;
}

double GetBufferValue(int bufferID, int index)
{
	switch (bufferID)
	{
	case RADIANCE_SUN_BREAKS:		return sunBreaks[index];
	case RADIANCE_ZENITH_BREAKS:	return zenithBreaks[index];
	case RADIANCE_EMPH_BREAKS:		return emphBreaks[index];
	case RADIANCE_VISIBLITIES:		return visibilitiesRad[index];
	case RADIANCE_ALBEDOS:			return albedosRad[index];
	case RADIANCE_ALTITUDES:		return altitudesRad[index];
	case RADIANCE_ELEVATIONS:		return elevationsRad[index];
	}

	return 0.0;
}

int GetBufferSize(int bufferID)
{
	switch (bufferID)
	{
	case RADIANCE_SUN_BREAKS:		return sunBreaks.length();
	case RADIANCE_ZENITH_BREAKS:	return zenithBreaks.length();
	case RADIANCE_EMPH_BREAKS:		return emphBreaks.length();
	case RADIANCE_VISIBLITIES:		return visibilitiesRad.length();
	case RADIANCE_ALBEDOS:			return albedosRad.length();
	case RADIANCE_ALTITUDES:		return altitudesRad.length();
	case RADIANCE_ELEVATIONS:		return elevationsRad.length();
	}

	return 0;
}

InterpolationParameter InterpolateBuffer(double queryVal, int bufferID)
{
	int bufferSize = GetBufferSize(bufferID);
	const double clamped = clamp(queryVal, GetBufferValue(bufferID, 0), GetBufferValue(bufferID, bufferSize - 1));

	int nextIndex = 1;
	for (; nextIndex < bufferSize; nextIndex++)
	{
		if (nextIndex == bufferSize || GetBufferValue(bufferID, nextIndex) > clamped)
			break;
	}

	InterpolationParameter parameter;
	parameter.index = nextIndex - 1;
	if (nextIndex >= bufferSize)
	{
		parameter.factor = 0.0;
	}
	else
	{
		double previousValue = GetBufferValue(bufferID, nextIndex - 1);
		double nextValue = GetBufferValue(bufferID, nextIndex);
		parameter.factor = (clamped - previousValue) / (nextValue - previousValue);
	}

	return parameter;
}

vec3 GetCubeMapCoords(vec2 size, uvec3 mapID)
{
	vec2 st = mapID.xy / size;
	vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

	vec3 coords;

	if		(mapID.z == 0)	coords = vec3(1.0, uv.y, -uv.x);
	else if (mapID.z == 1)	coords = vec3(-1.0, uv.y, uv.x);
	else if (mapID.z == 2)	coords = vec3(uv.x, 1.0, -uv.y);
	else if (mapID.z == 3)	coords = vec3(uv.x, -1.0, uv.y);
	else if (mapID.z == 4)	coords = vec3(uv.x, uv.y, 1.0);
	else if (mapID.z == 5)	coords = vec3(-uv.x, uv.y, -1.0);

	return normalize(coords);
}

Parameters ComputeParameters(
	vec3 viewpoint,
	vec3 viewDirection,
	float   groundLevelSolarElevationAtOrigin,
	float   groundLevelSolarAzimuthAtOrigin,
	float   visibility,
	float   albedo)
{
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
	// elev = 75, azim = 90
	// 0.25, 0.96, 0
	vec3 directionToSunN;
	directionToSunN.x = sin(groundLevelSolarAzimuthAtOrigin) * cos(groundLevelSolarElevationAtOrigin);
	directionToSunN.y = sin(groundLevelSolarElevationAtOrigin);
	directionToSunN.z = cos(groundLevelSolarAzimuthAtOrigin) * cos(groundLevelSolarElevationAtOrigin);

	directionToSunN = normalize(directionToSunN);

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

	vec3 shadowDirectionN;
    //shadowDirectionN.x = cos(shadowAngle) * cos(effectiveAzimuth);
	//shadowDirectionN.y = sin(shadowAngle);
	//shadowDirectionN.z = sin(shadowAngle) * cos(effectiveAzimuth);

    shadowDirectionN.x = cos(shadowAngle) * cos(effectiveAzimuth);
	shadowDirectionN.y = sin(shadowAngle);
	shadowDirectionN.z = cos(shadowAngle) * sin(effectiveAzimuth);


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

void InitializeParams(vec2 size, uvec3 mapID, out Parameters params)
{
	const vec3 viewDir = GetCubeMapCoords(size, mapID);
	params = ComputeParameters(u_OriginData.xyz, viewDir, u_Elevation, u_Azimuth, u_Visibility, u_Albedo);
}

float sRGBXYZtoHDR(float c) 
{
    c = c > 1 ? 1 : (c < 0 ? 0 : c);
    c = c <= 0.0031308 ? c * 12.92 : 1.055 * pow(c, 1. / 2.4) - 0.055;
    return c;
}

vec3 SpectrumToRGB(double spectrum[55])
{
	vec3 xyz = vec3(0.0);
	for (int wl = 0; wl < SPECTRUM_WAVELENGTHS.length(); wl++)
	{
		const int responseIdx = int((SPECTRUM_WAVELENGTHS[wl] - SPECTRAL_RESPONSE_START) / SPECTRAL_RESPONSE_STEP);
		if (0 <= responseIdx && responseIdx < SPECTRAL_RESPONSE.length())
			xyz = xyz + SPECTRAL_RESPONSE[responseIdx] * float(spectrum[wl]);
	}

	xyz = xyz * float(SPECTRUM_STEP);
	// XYZ to sRGB
	vec3 rgb;
	rgb.x = 3.2404542 * xyz.x - 1.5371385 * xyz.y - 0.4985314 * xyz.z;
	rgb.y = -0.9692660 * xyz.x + 1.8760108 * xyz.y + 0.0415560 * xyz.z;
	rgb.z = 0.0556434 * xyz.x - 0.2040259 * xyz.y + 1.0572252 * xyz.z;

	return vec3(
	    sRGBXYZtoHDR(rgb.x),
	    sRGBXYZtoHDR(rgb.y),
	    sRGBXYZtoHDR(rgb.z)
	);
}
