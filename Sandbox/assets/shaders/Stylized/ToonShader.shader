#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;

uniform vec3 u_CameraPosition;
uniform float u_Bias;
uniform float u_Scale;
uniform float u_Power;

out vec2 v_TexCoord;
out vec3 v_WorldNormal;
out vec3 v_WorldPosition;
out float v_FresnelAmount;

void main()
{
	vec3 worldPosition = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
	v_WorldPosition = worldPosition;
	vec3 worldNormal = vec3(u_ModelMatrix * vec4(a_Normal, 0.0));
	v_WorldNormal = worldNormal;
	vec3 i = normalize(worldPosition - u_CameraPosition);
	v_FresnelAmount = u_Bias + u_Scale * pow(1.0 + dot(i, worldNormal), u_Power);

	v_TexCoord = a_TexCoord;

	gl_Position = u_MVP * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;


struct Light
{
	vec3 Position;
	vec3 Color;
	float Intensity;
};


in vec2 v_TexCoord;
in vec3 v_WorldNormal;
in vec3 v_WorldPosition;
in float v_FresnelAmount;

uniform vec2 u_Scale;
uniform vec2 u_Offset;
uniform vec3 u_FresnelColor;
uniform vec3 u_RimColor;
uniform vec3 u_DiffuseColor;
uniform vec3 u_SpecularColor;
uniform vec3 u_AmbientColor;

uniform float u_SpecularStrength;
uniform float u_Shininess;
uniform float u_RimAmount;
uniform float u_RimThreshold;
uniform vec3 u_CameraPosition;

uniform sampler2D u_Texture;
uniform Light u_Light;


vec3 ToonShade(Light light, vec3 normal)
{
	vec3 directionToLight = normalize(light.Position - v_WorldPosition);
	vec3 viewDirection = normalize(u_CameraPosition - v_WorldPosition);

	float ndotl = max(dot(normal, directionToLight), 0.0);
	float intensity = smoothstep(0.0, 0.02, ndotl);

	vec3 halfVector = normalize(directionToLight + viewDirection);
	float ndoth = dot(normal, halfVector);
	float specularIntensity = pow(ndoth * intensity, u_Shininess * u_Shininess);
	float specularSmooth = smoothstep(0.005, 0.01, specularIntensity);

	vec3 samplerColor = texture(u_Texture, v_TexCoord).rgb;
	vec3 ambient = u_AmbientColor;
	vec3 specular = u_SpecularColor * specularSmooth * u_SpecularStrength;
	vec3 lightColor = light.Color * intensity;
	
	float rimDot = 1 - dot(viewDirection, normal);
	float rimIntensity = rimDot * pow(ndotl, u_RimThreshold);
	rimIntensity = smoothstep(u_RimAmount - 0.01, u_RimAmount + 0.01, rimIntensity);

	vec3 rimColor = rimIntensity * u_RimColor;

	return u_DiffuseColor * samplerColor * (ambient + specular + lightColor + rimColor);
}


void main()
{
	vec3 normal = normalize(v_WorldNormal);
	vec3 toonShadeResult = ToonShade(u_Light, normal);


	o_Color = vec4(toonShadeResult, 1.0);
}