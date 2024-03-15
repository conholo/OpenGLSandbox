#type compute
#version 450

//include Atmosphere/PragueDefinitions.include.glsl
//include Atmosphere/PragueBuffers.include.glsl
//include Atmosphere/PragueCommon.include.glsl
//include Atmosphere/PragueSkyRadiance.impl.glsl

vec3 CalculateSkyRadiance()
{
	Parameters params;
	InitializeParams(imageSize(o_Cubemap), gl_GlobalInvocationID, params);

    double spectrum[55];
	for (int i = 0; i < 55; i++)
		spectrum[i] = EvaluateSkyRadiance(params, SPECTRUM_WAVELENGTHS[i]);
		
    vec3 rgb = SpectrumToRGB(spectrum);
    return rgb;
}

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	vec3 Result = CalculateSkyRadiance();
	imageStore(o_Cubemap, ivec3(gl_GlobalInvocationID), vec4(Result, 1.0));
}
