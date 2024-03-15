#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

out vec2 v_TexCoord;
out vec3 v_WorldSpaceViewDirection;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;

void main()
{
	v_TexCoord = a_TexCoord;
	vec3 viewVector = (u_InverseProjection * vec4(a_TexCoord * 2.0 - 1.0, 0.0, 1.0)).xyz;
	v_WorldSpaceViewDirection = (u_InverseView * vec4(viewVector, 0.0)).xyz;
	v_WorldSpaceViewDirection.z *= -1.0;
	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_TransmittanceOutput;
layout(location = 1) out vec4 o_SunRadianceOutput;

in vec2 v_TexCoord;
in vec3 v_WorldSpaceViewDirection;

uniform sampler2D u_DepthTexture;
uniform float u_TransmittanceScale;
uniform float u_SunRadianceTransmittanceScale;
uniform float u_TotalSunRadianceModifier;
uniform float u_Near;
uniform float u_Far;

layout(std430, binding = 9) buffer TransV
{
	float dataTransV[];
};

layout(std430, binding = 10) buffer TransU
{
	float dataTransU[];
};

layout(std430, binding = 11) buffer AltitudeTrans
{
	double altitudesTrans[];
};

layout(std430, binding = 12) buffer VisibilitiesTrans
{
	double visibilitiesTrans[];
};

//include Atmosphere/PragueDefinitions.include.glsl
//include Atmosphere/PragueBuffers.include.glsl
//include Atmosphere/PragueCommon.include.glsl
//include Atmosphere/PragueTransmittance.impl.glsl
//include Atmosphere/PragueSunRadiance.impl.glsl

float LinearizeDepth(float depth)
{ 
	float near = u_Near;
	float far = u_Far;

	float ndc = depth * 2.0 - 1.0;
	return 2.0 * near * far / (far + near - ndc * (far - near));
}

void main()
{
	// Normalized world space view direction towards the current pixel.
	const vec3 RayDirection = normalize(v_WorldSpaceViewDirection);
	// [0:1] Depth from depth buffer.
	float Depth = texture(u_DepthTexture, v_TexCoord).r;
	// Linearize the depth.
	float LinearDepth = LinearizeDepth(Depth);
	// Compute atmosperic parameters.
	Parameters Params = ComputeParameters(u_OriginData.xyz, RayDirection, u_Elevation, u_Azimuth, u_Visibility, u_Albedo);

	// Calculate the spectral values for the transmittance along the path to this pixel given the relative distance.
	double TransmittanceSpectrum[55];
	double SunRadianceSpectrum[55];
	for (int i = 0; i < 55; i++)
	{
        TransmittanceSpectrum[i] = CalculateTransmittance(Params, SPECTRUM_WAVELENGTHS[i], LinearDepth) * u_TransmittanceScale;
        SunRadianceSpectrum[i] = CalculateSunRadiance(Params, SPECTRUM_WAVELENGTHS[i], LinearDepth, u_SunRadianceTransmittanceScale);
	}

	vec3 T = SpectrumToRGB(TransmittanceSpectrum);
	vec3 SR = SpectrumToRGB(SunRadianceSpectrum) * u_TotalSunRadianceModifier;
	o_SunRadianceOutput = vec4(SR, 1.0f);
	o_TransmittanceOutput = vec4(T, 1.0f);
}