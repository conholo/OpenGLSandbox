double CalculateSunRadiance(Parameters params, const double wavelength, const float distance, float factor)
{
	// Ignore wavelengths outside the dataset range.
	if (wavelength < SUN_RAD_START || wavelength >= SUN_RAD_END)
        return 0.0;

	// Return zero for rays not hitting the sun.
	if (params.gamma > SUN_RADIUS)
        return 0.0;

	// Compute index into the sun radiance table.
	const double idx = (wavelength - SUN_RAD_START) / SUN_RAD_STEP;

	const int    idxInt = int(floor(idx));
	const double idxFloat = idx - floor(idx);

	// Interpolate between the two closest values in the sun radiance table.
	const double sunRadiance = SUN_RAD_TABLE[idxInt] * (1.0 - idxFloat) + SUN_RAD_TABLE[idxInt + 1] * idxFloat;
	//assert(sunRadiance > 0.0);

	// Compute transmittance towards the sun.
	const double tau = CalculateTransmittance(params, wavelength, FLT_MAX) * factor;
	
	// Combine.
	return sunRadiance * tau;
}