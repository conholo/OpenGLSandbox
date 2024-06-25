#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 ViewProjectionMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;

struct VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;

	//vec3 ShadowMapCoords;
	vec3 ViewPosition;
};

layout(location = 0) out VertexOutput Output;

void main()
{
	vec4 worldPosition = ModelMatrix * vec4(a_Position, 1.0);
	Output.WorldPosition = worldPosition.xyz;
	Output.Normal = mat3(ModelMatrix) * a_Normal;
	Output.TexCoord = a_TexCoord;
	Output.WorldNormals = mat3(ModelMatrix) * mat3(a_Tangent, a_Binormal, a_Normal);
	Output.WorldTransform = mat3(ModelMatrix);
	Output.Binormal = a_Binormal;
    Output.ViewPosition = vec3(ViewMatrix * vec4(Output.WorldPosition, 1.0));
	//Output.ShadowMapCoords = u_ShadowMatrix * vec4(Output.WorldPosition, 1.0);

	gl_Position = ViewProjectionMatrix * ModelMatrix * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_MainColor;
layout(location = 1) out vec4 o_SceneRadianceSample;

struct VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;

	//vec3 ShadowMapCoords;
	vec3 ViewPosition;
};

layout(location = 0) in VertexOutput Input;

const float PI = 3.141592;
const float Epsilon = 0.00001;
const vec3 FresnelDialectric = vec3(0.04);

uniform vec3 LightPosition;
uniform vec3 LightColor;
uniform float LightIntensity;

uniform vec3 AlbedoColor;
uniform float Metalness;
uniform float Roughness;
uniform float Emission = 0.0;
uniform int UseNormalMap;

uniform float EnvironmentMapIntensity = 1.0;

uniform vec3 CameraPosition;

// PBR texture inputs
uniform sampler2D sampler_AlbedoTexture;
uniform sampler2D sampler_NormalTexture;
uniform sampler2D sampler_MetalnessTexture;
uniform sampler2D sampler_RoughnessTexture;

uniform samplerCube sampler_RadianceCube;
uniform samplerCube sampler_IrradianceCube;
uniform sampler2D sampler_BRDFLUT;
//uniform sampler2D sampler_ShadowMap;


struct PBRParameters
{
	vec3 Albedo;
	float Roughness;
	float Metalness;

	vec3 Normal;
	vec3 View;
	float NdotV;
} PBRParams;


//float ShadowCalculation(vec4 fragPositionLightSpace, vec3 lightDirection, vec3 normal)
//{
//	// Perform perspective divide
//	// Light-space position in the range of [-1, 1]
//	// This is meaningless if we're using an orthographic projection for the light,
//	// as the w component is untouched, but becomes important when using a perspective
//	// projection
//	vec3 projectionCoords = fragPositionLightSpace.xyz / fragPositionLightSpace.w;
//
//	// Transform NDC to [0, 1], the depth map is in the range [0, 1]
//	projectionCoords = projectionCoords * 0.5 + 0.5;
//
//	// Now that the coordinates are in the range [0, 1], we can sample from the depth map
//	// The closest depth from the light's perspective [0, 1]
//	float closestDepth = texture(u_ShadowMap, projectionCoords.xy).r;
//	// The actual depth of the fragment from the light's perspective
//	float currentDepth = projectionCoords.z;
//
//	float bias = max(0.05 * (1.0 - dot(normal, lightDirection)), 0.005);
//
//	// Simple shadows
//	// If the current depth is greater than the closest, the fragment is in shadow.
//	//float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
//
//	//// PCF (percentage-closer filtering)
//	float shadow = 0.0;
//	vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
//	for (int x = -1; x <= 1; x++)
//	{
//		for (int y = -1; y <= 1; y++)
//		{
//			// The depth at neighboring texels.
//			float pcfDepth = texture(u_ShadowMap, projectionCoords.xy + vec2(x, y) * texelSize).r;
//
//			// Apply bias to remove any shadow acne from this sample.
//			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
//		}
//	}
//	
//	// All adjacent neighbors + this sample = 9 samples.
//	shadow /= 9.0;
//	
//
//	// If the projected coordinate exists outside of the Light's Far plane, set the shadow to zero, so 
//	// a shadow isn't set.  If the projection coordinate zed value is greater than 1, we'll always detect
//	// shadow.  In this case, the only reason this occurs is because the fragment lies outside of the bounds of
//	// the shadow map.
//	// Checking that the coordinate exists within the far plane limits and clamping the shadow map texture
//	// prevents oversampling.
//	if (projectionCoords.z > 1.0)
//		shadow = 0.0;
//
//	return shadow;
//}

vec3 FresnelSchlick(vec3 F0, float cosTheta, float roughness)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float NDFGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSquared = alpha * alpha;

	float denominator = (cosLh * cosLh) * (alphaSquared - 1.0) + 1.0;
	return alphaSquared / (PI * denominator * denominator);
}

float GaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float GaSchlickGGX(float cosLi, float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return GaSchlickG1(cosLi, k) * GaSchlickG1(NdotV, k);
}

vec3 RotateVectorAboutY(float angle, vec3 vec)
{
	angle = radians(angle);
	mat3x3 rotationMatrix = 
	    { 
	        vec3(cos(angle),    0.0,    sin(angle)),
            vec3(0.0,           1.0,    0.0),
            vec3(-sin(angle),   0.0,    cos(angle)) 
        };
	return rotationMatrix * vec;
}

vec3 IBL()
{
    const float EnvMapRotation = 0.0;

	vec3 Lr = 2.0 * PBRParams.NdotV * PBRParams.Normal - PBRParams.View;
	// Fresnel reflectance, metals use albedo
	vec3 F0 = mix(FresnelDialectric, PBRParams.Albedo, PBRParams.Metalness);
    
	vec3 irradiance = texture(sampler_IrradianceCube, PBRParams.Normal).rgb;
	vec3 F = FresnelSchlickRoughness(F0, PBRParams.NdotV, PBRParams.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - PBRParams.Metalness);
	vec3 diffuseIBL = PBRParams.Albedo * irradiance;

	int envRadianceTexLevels = textureQueryLevels(sampler_RadianceCube);
	float NoV = clamp(PBRParams.NdotV, 0.0, 1.0);
	vec3 R = 2.0 * dot(PBRParams.View, PBRParams.Normal) * PBRParams.Normal - PBRParams.View;
	vec3 specularIrradiance = textureLod(
	            sampler_RadianceCube, 
	            RotateVectorAboutY(EnvMapRotation, Lr), 
	            envRadianceTexLevels * Roughness).rgb;

	vec2 specularBRDF = texture(sampler_BRDFLUT, vec2(PBRParams.NdotV, PBRParams.Roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F0 * specularBRDF.x + specularBRDF.y);

	return kd * diffuseIBL + specularIBL;
}

vec3 CalculateLighting()
{
	vec4 AlbedoTexColor = texture(sampler_AlbedoTexture, Input.TexCoord);
	PBRParams.Albedo = AlbedoTexColor.rgb * AlbedoColor;
	float Alpha = AlbedoTexColor.a;

	PBRParams.Metalness = texture(sampler_MetalnessTexture, Input.TexCoord).r * Metalness;
	PBRParams.Roughness = texture(sampler_RoughnessTexture, Input.TexCoord).r * Roughness;
	PBRParams.Roughness = max(PBRParams.Roughness, 0.05); 

	PBRParams.Normal = normalize(Input.Normal);
	if (UseNormalMap == 1)
	{
		PBRParams.Normal = normalize(texture(sampler_NormalTexture, Input.TexCoord).rgb * 2.0f - 1.0f);
		PBRParams.Normal = normalize(Input.WorldNormals * PBRParams.Normal);
	}
	
	PBRParams.View = normalize(CameraPosition - Input.WorldPosition);
	PBRParams.NdotV = max(0.0, dot(PBRParams.Normal, PBRParams.View));

	// Specular reflection vector
	vec3 Lr = 2.0 * PBRParams.NdotV * PBRParams.Normal - PBRParams.View;
	// Fresnel reflectance, metals use albedo
	vec3 F0 = mix(FresnelDialectric, PBRParams.Albedo, PBRParams.Metalness);

	vec3 Li = normalize(LightPosition - Input.WorldPosition);
    vec3 LRadiance = LightColor * LightIntensity;
    vec3 Lh = normalize(Li + PBRParams.View);

	float cosLi  = max(0.0, dot(PBRParams.Normal, Li));
	float cosLh  = max(0.0, dot(PBRParams.Normal, Lh));
	float cosLhV = max(0.0, dot(Lh, PBRParams.View));

	vec3  F = FresnelSchlick(F0, cosLhV, PBRParams.Roughness);
	float D = NDFGGX(cosLh, PBRParams.Roughness);
	float G = GaSchlickGGX(cosLi, PBRParams.NdotV, PBRParams.Roughness);

	vec3 kD = (1.0 - F) * (1.0 - PBRParams.Metalness);
	vec3 diffuseBRDF = kD * PBRParams.Albedo;

	// Cook-Torrance
	vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * PBRParams.NdotV);
    specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));

	vec3 result = (diffuseBRDF + specularBRDF) * LRadiance * cosLi;
	return result;
}

void main()
{
	//float shadow = ShadowCalculation(Input.ShadowMapCoords, lightDirection, normal);
	vec3 Lighting = CalculateLighting();
	Lighting += PBRParams.Albedo * Emission;
    vec3 IBLContribution = IBL() * EnvironmentMapIntensity;
    o_SceneRadianceSample = vec4(IBLContribution, 1.0);
	o_MainColor = vec4(Lighting + IBLContribution, 1.0);
}
