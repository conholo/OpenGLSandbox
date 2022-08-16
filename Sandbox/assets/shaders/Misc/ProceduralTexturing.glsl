#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 u_ModelMatrix;
uniform mat4 u_MVP;

out vec3 v_WorldPosition;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main()
{
	v_WorldPosition = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
	v_Normal = vec3(u_ModelMatrix * vec4(a_Normal, 0.0));
	v_TexCoord = a_TexCoord;

	gl_Position = u_MVP * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
#define PI 3.14159265359

layout(location = 0) out vec4 o_Color;

uniform float u_ElapsedTime;

uniform bool u_UseNoise;
uniform float u_NoiseFrequency;
uniform float u_NoiseAmplitude;

uniform float u_AlphaPercent;
uniform float u_Ad;
uniform float u_Bd;
uniform float u_Tol;
uniform int u_Factor = 2;

uniform float u_Shininess;
uniform float u_AmbientStrength;
uniform float u_SpecularStrength;
uniform float u_DiffuseStrength;

in vec2 v_TexCoord;
in vec3 v_WorldPosition;
in vec3 v_Normal;

uniform int u_Animate;
uniform vec3 u_BGColor;
uniform vec3 u_DotColor;
uniform vec3 u_CameraPosition;
uniform vec3 u_LightPosition;

uniform sampler2D u_NoiseTexture;
uniform sampler2D u_Texture;


float Wave(float a, float f, float v)
{
	return a * sin(u_ElapsedTime + f * v * PI);
}

float LoopPow(float x, int p)
{
	float result = x;
	for (int i = 0; i < p - 1; i++)
		result *= x;

	return result;
}

void main()
{
	vec3 normal = normalize(v_Normal);
	float Ar = u_Ad / 2.0;
	float Br = u_Bd / 2.0;

	float animation = u_Animate == 1 ? 1.0 : 0.0;
	vec2 st = v_TexCoord + vec2(Wave(0.5, 1.5, v_TexCoord.t), Wave(0.5, 1.5, v_TexCoord.s)) * animation;
	float noise = u_UseNoise ? u_NoiseAmplitude * texture(u_NoiseTexture, st * u_NoiseFrequency).r * 2.0 - 1.0 : 0.0;

	float s = st.x;
	float t = st.y;

	// Find grid ID for the uv.
	int cellX = int(s / u_Ad);
	int cellY = int(t / u_Bd);

	// Find the lower left coordinate of the grid cell for the uv.
	float cellGridCoordX = float(cellX) * u_Ad;
	float cellGridCoordY = float(cellY) * u_Bd;

	// Calculate vectors wrt to ellipse center.
	float sc = cellGridCoordX + Ar;
	float ds = s - sc;
	float tc = cellGridCoordY + Br;
	float dt = t - tc;

	// Calculate the distance 
	float oldDistance = sqrt(ds * ds + dt * dt);
	float newDistance = oldDistance + noise;
	float scale = newDistance / oldDistance;

	// Get the noisy offsets.
	float dx = ds * scale / Ar;
	float dy = dt * scale / Br;

	// Recalulate the distance based on the transformed offset vectors.
	float d = dx * dx + dy * dy;

	bool isNonEllipse = d > 1.0 + u_Tol;

	if (isNonEllipse && u_AlphaPercent == 0.0)
		discard;

	float pct = smoothstep(1.0 - u_Tol, 1.0 + u_Tol, d);

	vec3 color = mix(u_DotColor, u_BGColor, pct) * texture(u_Texture, v_TexCoord).rgb;

	vec3 lightDirection = normalize(u_LightPosition - v_WorldPosition);
	vec3 viewDirection = normalize(u_CameraPosition - v_WorldPosition);
	float diffuse = max(0.0, dot(normal, lightDirection));

	vec3 halfway = normalize(lightDirection + viewDirection);
	float specular = pow(max(dot(normal, halfway), 0.0), u_Shininess * u_Shininess);

	vec3 diffuseColor = color * diffuse * u_DiffuseStrength;
	vec3 ambientColor = color * u_AmbientStrength;
	vec3 specularColor = specular * vec3(1.0) * u_SpecularStrength;

	vec3 result = diffuseColor + ambientColor + specularColor;

	o_Color = vec4(result, isNonEllipse ? u_AlphaPercent : 1.0);
}