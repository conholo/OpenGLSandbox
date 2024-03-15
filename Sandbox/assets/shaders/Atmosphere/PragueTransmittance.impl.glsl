int GetCoefficientSpaceOffsetTrans(int visibility, int altitude, int wavelength)
{
	return  ((visibility * altitudesTrans.length() + altitude) * u_Channels + wavelength) * u_RankTrans;
}

int GetCoefficientSpaceOffsetTransBase(const int altitude, const int a, const int d)
{
	return altitude * u_aDim * u_dDim * u_RankTrans + (d * u_aDim + a) * u_RankTrans;
}

/// Intersects the given ray (assuming rayPosX == 0) with a circle at origin with the given radius.
/// return In the case the ray intersects the circle, distance to the circle is returned, otherwise the
///         function returns negative number.
double IntersectRayWithCircle2D(double rayDirX, double rayDirY, double rayPosY, double circleRadius)
{
	const double qA = rayDirX * rayDirX + rayDirY * rayDirY;
	const double qB = 2.0 * rayPosY * rayDirY;
	const double qC = rayPosY * rayPosY - circleRadius * circleRadius;
	double       discrim = qB * qB - 4.0 * qA * qC;

	// No intersection or touch only
	if (discrim <= 0.0)
		return -1.0;

	discrim = sqrt(discrim);

	// Compute distances to both intersections
	const double d1 = (-qB + discrim) / (2.0 * qA);
	const double d2 = (-qB - discrim) / (2.0 * qA);

	// Try to take the nearest positive one
	const double distToIsect = (d1 > 0.0 && d2 > 0.0) ? min(d1, d2) : max(d1, d2);
	return distToIsect;
}

/// Auxiliary function for \ref toTransmittanceParams. Computes [altitude, distance] parameters from
/// coordinates of intersection of view ray and ground or atmosphere edge. Note: using floats here instead of
/// doubles will cause banding artifacts.
double[2] IntersectToAltitudeDistance(const double isectX, const double isectY)
{
	// Distance to the intersection from world origin (not along ray as distToIsect in the calling method).
	const double isectDist = sqrt(isectX * isectX + isectY * isectY);
	//assert(isectDist > 0.0);

	// Compute normalized and non-linearly scaled position in the atmosphere
	double altitude = clamp(isectDist - PLANET_RADIUS, 0.0, ATMOSPHERE_WIDTH);
	altitude = pow(float(altitude) / ATMOSPHERE_WIDTH, 1.0 / 3.0);
	double distance = acos(float(isectY) / float(isectDist)) * PLANET_RADIUS;
	distance = sqrt(distance / DIST_TO_EDGE);
	distance = sqrt(distance); // Calling twice sqrt, since it is faster than pow(...,0.25)
	distance = min(1.0, distance);
	//assert(0.0 <= altitude && altitude <= 1.0);
	//assert(0.0 <= distance && distance <= 1.0);

	return double[2](altitude, distance);
}

double nonlerp(const double a, const double b, const double w, const double p)
{
	const double c1 = pow(float(a), float(p));
	const double c2 = pow(float(b), float(p));
	return ((pow(float(w), float(p)) - c1) / (c2 - c1));
}

InterpolationParameter GetInterpolationParameterTrans(const double value, const int paramCount, const int power)
{
	InterpolationParameter param;
	param.index = min(int(value * paramCount), paramCount - 1);
	param.factor = 0.0;
	if (param.index < paramCount - 1)
	{
		param.factor = nonlerp(double(param.index) / paramCount, double(param.index + 1) / paramCount, value, power);
		param.factor = clamp(param.factor, 0.0, 1.0);
	}
	return param;
}

TransmittanceParameters ToTransmittanceParams(double theta, double distance, double altitude)
{
	const double rayDirX = sin(float(theta));
	const double rayDirY = cos(float(theta));
	const double rayPosY = PLANET_RADIUS + altitude; // rayPosX == 0

	double ATMOSPHERE_EDGE = PLANET_RADIUS + ATMOSPHERE_WIDTH;

	// Find intersection of the ground-to-sun ray with edge of the atmosphere (in 2D)
	double distToIsect = -1.0;
	double LOW_ALTITUDE = 0.3;
	if (altitude < LOW_ALTITUDE)
	{
		// Special handling of almost zero altitude case to avoid numerical issues.
		distToIsect = theta <= 0.5 * PI ? IntersectRayWithCircle2D(rayDirX, rayDirY, rayPosY, ATMOSPHERE_EDGE) : 0.0;
	}
	else
	{
		distToIsect = IntersectRayWithCircle2D(rayDirX, rayDirY, rayPosY, PLANET_RADIUS);
		if (distToIsect < 0.0)
			distToIsect = IntersectRayWithCircle2D(rayDirX, rayDirY, rayPosY, ATMOSPHERE_EDGE);
	}
	// The ray should always hit either the edge of the atmosphere or the planet (we are starting inside the
	// atmosphere).
	//assert(distToIsect >= 0.0);

	distToIsect = min(distToIsect, distance);

	// Compute intersection coordinates
	const double isectX = rayDirX * distToIsect;
	const double isectY = rayDirY * distToIsect + rayPosY;

	// Get the internal vec2(altitude, distance) parameters - may need to be converted to double precision in the event of banding
	double alt[2] = IntersectToAltitudeDistance(isectX, isectY);

	// Convert to interpolation parameters
	TransmittanceParameters params;
	params.altitude = GetInterpolationParameterTrans(alt[0], u_aDim, 3);
	params.distance = GetInterpolationParameterTrans(alt[1], u_dDim, 4);

	return params;
}

double ReconstructTransmittance(int visibilityIndex, int altitudeIndex, TransmittanceParameters transParams, int channelIndex)
{
	int coefficientOffset = GetCoefficientSpaceOffsetTrans(visibilityIndex, altitudeIndex, channelIndex);

	// Load transmittance values for bi-linear interpolation
	double transmittance[4] = { 0.0, 0.0, 0.0, 0.0 };
	int                   index = 0;
	for (int a = transParams.altitude.index; a <= transParams.altitude.index + 1; ++a)
	{
		if (a < u_aDim)
		{
			for (int d = transParams.distance.index; d <= transParams.distance.index + 1; ++d)
			{
				if (d < u_dDim)
				{
					int baseCoefficients = GetCoefficientSpaceOffsetTransBase(altitudeIndex, a, d);
					for (int i = 0; i < u_RankTrans; ++i)
					{
						// Reconstruct transmittance value
						transmittance[index] += double(dataTransU[baseCoefficients + i]) * double(dataTransV[coefficientOffset + i]);
					}
					index++;
				}
			}
		}
	}

	// Perform bi-linear interpolation
	if (transParams.distance.factor > 0.f)
	{
		transmittance[0] = mix(transmittance[0], transmittance[1], transParams.distance.factor);
		transmittance[1] = mix(transmittance[2], transmittance[3], transParams.distance.factor);
	}
	transmittance[0] = max(transmittance[0], 0.0);
	
	if (transParams.altitude.factor > 0.f)
	{
		transmittance[1] = max(transmittance[1], 0.0);
		transmittance[0] = mix(transmittance[0], transmittance[1], transParams.altitude.factor);
	}

	//assert(transmittance[0] >= 0.0);
	return transmittance[0];
}

double InterpolateTransmittance(int visibilityIndex, InterpolationParameter altitudeParam, TransmittanceParameters transParams, int channelIndex)
{
	// Get transmittance for the nearest lower altitude.
	double trans = ReconstructTransmittance(visibilityIndex, altitudeParam.index, transParams, channelIndex);

	// Interpolate with transmittance for the nearest higher altitude if needed.
	if (altitudeParam.factor > 0.0)
	{
		const double transHigh = ReconstructTransmittance(visibilityIndex, altitudeParam.index + 1, transParams, channelIndex);
		trans = mix(trans, transHigh, altitudeParam.factor);
	}

	return trans;
}

InterpolationParameter InterpolateAltitudesTransmittance(double queryVal)
{
	const double clamped = clamp(queryVal, altitudesTrans[0], altitudesTrans[altitudesTrans.length() - 1]);

	int nextIndex = 1;
	for (; nextIndex <= altitudesTrans.length(); nextIndex++)
	{
		if (nextIndex == altitudesTrans.length() || altitudesTrans[nextIndex] > clamped)
			break;
	}

	InterpolationParameter parameter;
	parameter.index = nextIndex - 1;
	if (nextIndex >= altitudesTrans.length())
	{
		parameter.factor = 0.0;
	}
	else
	{
		double previousValue = altitudesTrans[nextIndex - 1];
		double nextValue = altitudesTrans[nextIndex];
		parameter.factor = (clamped - previousValue) / (nextValue - previousValue);
	}

	return parameter;
}

InterpolationParameter InterpolateVisibilitiesTransmittance(double queryVal)
{
	const double clamped = clamp(queryVal, visibilitiesTrans[0], visibilitiesTrans[visibilitiesTrans.length() - 1]);

	int nextIndex = 1;
	for (; nextIndex < visibilitiesTrans.length(); nextIndex++)
	{
		if (nextIndex == visibilitiesTrans.length() || visibilitiesTrans[nextIndex] > clamped)
			break;
	}

	InterpolationParameter parameter;
	parameter.index = nextIndex - 1;
	if (nextIndex >= visibilitiesTrans.length())
	{
		parameter.factor = 0.0;
	}
	else
	{
		double previousValue = visibilitiesTrans[nextIndex - 1];
		double nextValue = visibilitiesTrans[nextIndex];
		parameter.factor = (clamped - previousValue) / (nextValue - previousValue);
	}

	return parameter;
}

double CalculateTransmittance(Parameters params, double wavelength, double distance)
{
	// Ignore wavelengths outside the dataset range.
	if (wavelength < u_ChannelStart || wavelength >= (u_ChannelStart + u_Channels * u_ChannelWidth))
		return 0.0;

	// Don't interpolate wavelengths inside the dataset range.
	const int channelIndex = int(floor((wavelength - u_ChannelStart) / u_ChannelWidth));

	// Translate configuration values to indices and interpolation factors.
	const InterpolationParameter visibilityParam = InterpolateVisibilitiesTransmittance(params.visibility);
	const InterpolationParameter altitudeParam = InterpolateAltitudesTransmittance(params.altitude);

	// Calculate position in the atmosphere.
	const TransmittanceParameters transParams = ToTransmittanceParams(params.theta, distance, params.altitude);
	// Get transmittance for the nearest lower visibility.
	double trans = InterpolateTransmittance(visibilityParam.index, altitudeParam, transParams, channelIndex);

	// Interpolate with transmittance for the nearest higher visibility if needed.
	if (visibilityParam.factor > 0.0)
	{
		const double transHigh = InterpolateTransmittance(visibilityParam.index + 1, altitudeParam, transParams, channelIndex);
		trans = mix(trans, transHigh, visibilityParam.factor);
	}

	return trans * trans;
}