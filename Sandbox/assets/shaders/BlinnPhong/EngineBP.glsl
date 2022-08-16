#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform Global
{
	float Elapsed;
	float DeltaTime;
};

layout(std140, binding = 1) uniform Camera
{
	mat4 u_ViewProjectionMatrix;
	mat4 u_ModelMatrix;
	mat4 u_NormalMatrix;
};

layout(std140, binding = 2) uniform Shadow
{
	mat4 u_ShadowMatrix;
};

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_WorldPosition;
out vec4 v_ShadowCoords;

void main()
{
	v_WorldPosition = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
	v_Normal = vec3(u_NormalMatrix * vec4(a_Normal, 0.0));
	v_TexCoord = a_TexCoord;
	v_ShadowCoords = u_ShadowMatrix * vec4(v_WorldPosition, 1.0);

	gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct MaterialProperties
{
	vec3 DiffuseColor;
	vec3 AmbientColor;
	float AmbientStrength;
	float DiffuseStrength;
	float SpecularStrength;
	float Shininess;
};

struct DirectionalLight
{
	vec3 Position;
	vec3 Color;
	float Intensity;
};

uniform MaterialProperties u_MaterialProperties;
uniform DirectionalLight u_DirectionalLight;
uniform vec3 u_CameraPosition;

uniform sampler2D u_Texture;
uniform sampler2D u_ShadowMap;

in vec3 v_WorldPosition;
in vec3 v_Normal;
in vec2 v_TexCoord;
in vec4 v_ShadowCoords;

float ShadowCalculation(vec4 fragPositionLightSpace, vec3 lightDirection, vec3 normal)
{
	// Perform perspective divide
	// Light-space position in the range of [-1, 1]
	// This is meaningless if we're using an orthographic projection for the light,
	// as the w component is untouched, but becomes important when using a perspective
	// projection
	vec3 projectionCoords = fragPositionLightSpace.xyz / fragPositionLightSpace.w;

	// Transform NDC to [0, 1], the depth map is in the range [0, 1]
	projectionCoords = projectionCoords * 0.5 + 0.5;

	// Now that the coordinates are in the range [0, 1], we can sample from the depth map
	// The closest depth from the light's perspective [0, 1]
	float closestDepth = texture(u_ShadowMap, projectionCoords.xy).r;
	// The actual depth of the fragment from the light's perspective
	float currentDepth = projectionCoords.z;

	float bias = max(0.05 * (1.0 - dot(normal, lightDirection)), 0.005);

	// Simple shadows
	// If the current depth is greater than the closest, the fragment is in shadow.
	//float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	

	//// PCF (percentage-closer filtering)
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			// The depth at neighboring texels.
			float pcfDepth = texture(u_ShadowMap, projectionCoords.xy + vec2(x, y) * texelSize).r;

			// Apply bias to remove any shadow acne from this sample.
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	
	// All adjacent neighbors + this sample = 9 samples.
	shadow /= 9.0;
	

	// If the projected coordinate exists outside of the Light's Far plane, set the shadow to zero, so 
	// a shadow isn't set.  If the projection coordinate zed value is greater than 1, we'll always detect
	// shadow.  In this case, the only reason this occurs is because the fragment lies outside of the bounds of
	// the shadow map.
	// Checking that the coordinate exists within the far plane limits and clamping the shadow map texture
	// prevents oversampling.
	if (projectionCoords.z > 1.0)
		shadow = 0.0;

	return shadow;
}

vec3 BlinnPhong(vec3 normal, DirectionalLight light, MaterialProperties properties)
{
	vec3 lightDirection = normalize(light.Position - v_WorldPosition);
	vec3 viewDirection = normalize(u_CameraPosition - v_WorldPosition);

	// Ambient
	vec3 ambient = light.Intensity * mix(light.Color, properties.AmbientColor, properties.AmbientStrength);

	// Diffuse
	float NdotL = max(dot(normal, lightDirection), 0);
	vec3 diffuse = light.Intensity * NdotL * properties.DiffuseStrength * light.Color;

	// Specular
	vec3 halfway = normalize(lightDirection + viewDirection);
	float specular = pow(max(dot(normal, halfway), 0.0), properties.Shininess * properties.Shininess);
	vec3 spec = light.Intensity * specular * light.Color * properties.SpecularStrength;

	float shadow = ShadowCalculation(v_ShadowCoords, lightDirection, normal);

	// If the fragment is in shadow, we still want to apply the ambient shading.
	return (ambient + (1.0 - shadow) * (diffuse + spec)) * properties.DiffuseColor;;
}

void main()
{
	vec3 normal = normalize(v_Normal);

	vec3 result = BlinnPhong(normal, u_DirectionalLight, u_MaterialProperties);
	vec3 albedo = texture(u_Texture, v_TexCoord).xyz;

	albedo *= result;

	o_Color = vec4(albedo, 1.0);
}