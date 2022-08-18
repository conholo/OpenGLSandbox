// Evaluates piecewise linear approximation
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
	for (int r = 0; r < u_RadRank; ++r)
	{
		// Restore the right value in the 'sun' vector
		int sunBaseOffset = baseChannelOffset + sunOffset + r * sunStride + radianceParameters.gamma.index;
		const double sunParam = evalPL(sunBaseOffset, radianceParameters.gamma.factor);
		memoryBarrier();

		// Restore the right value in the 'zenith' vector
		int zenithBaseOffset = baseChannelOffset + zenithOffset + r * zenithStride + radianceParameters.alpha.index;
		const double zenithParam = evalPL(zenithBaseOffset, radianceParameters.alpha.factor);
		memoryBarrier();

		// Accumulate their "outer" product
		result += sunParam * zenithParam;
	}

	// De-emphasize (for radiance only)
	if (emphBreaks.length() > 0)
	{
		int emphBaseOffset = baseChannelOffset + emphOffset + radianceParameters.zero.index;
		const double emphParam = evalPL(emphBaseOffset, radianceParameters.zero.factor);
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
	angleParameters.gamma = InterpolateBuffer(params.gamma, RADIANCE_SUN_BREAKS);

	if (emphBreaks.length() > 0)
	{ 
		angleParameters.alpha = InterpolateBuffer(params.elevation < 0.0 ? params.shadow : params.zero, RADIANCE_ZENITH_BREAKS);
		angleParameters.zero = InterpolateBuffer(params.zero, RADIANCE_EMPH_BREAKS);
	}
	else
	{ // for polarisation
		angleParameters.alpha = InterpolateBuffer(params.theta, RADIANCE_ZENITH_BREAKS);
	}
	
	const InterpolationParameter visibilityParam = InterpolateBuffer(params.visibility, RADIANCE_VISIBLITIES);
	const InterpolationParameter albedoParam = InterpolateBuffer(params.albedo, RADIANCE_ALBEDOS);
	const InterpolationParameter altitudeParam = InterpolateBuffer(params.altitude, RADIANCE_ALTITUDES);
	const InterpolationParameter elevationParam = InterpolateBuffer(radiansToDegrees(params.elevation), RADIANCE_ELEVATIONS);
	
	ControlParameters controlParameters;
	for (int i = 0; i < 16; ++i)
	{
		const int visibilityIndex = min(visibilityParam.index + i / 8, int(visibilitiesRad.length() - 1));
		const int albedoIndex =		min(albedoParam.index + (i % 8) / 4, int(albedosRad.length() - 1));
		const int altitudeIndex =	min(altitudeParam.index + (i % 4) / 2, int(altitudesRad.length() - 1));
		const int elevationIndex =	min(elevationParam.index + i % 2, int(elevationsRad.length() - 1));
	
		controlParameters.coefficientOffsets[i] = 
			GetCoefficientSpaceOffset(u_TotalCoefsSingleConfigRad, elevationIndex, altitudeIndex, visibilityIndex, albedoIndex, channelIndex);
	}
	memoryBarrier();

	controlParameters.interpolationFactors[0] = visibilityParam.factor;
	controlParameters.interpolationFactors[1] = albedoParam.factor;
	controlParameters.interpolationFactors[2] = altitudeParam.factor;
	controlParameters.interpolationFactors[3] = elevationParam.factor;
	
	const double result = Interpolate(angleParameters, controlParameters);
	return max(0.0, result);
}
