#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

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

uniform float u_Ad;
uniform float u_Bd;
uniform float u_Tol;

uniform float u_Shininess;
uniform float u_AmbientStrength;

in vec2 v_TexCoord;
in vec3 v_WorldPosition;
in vec3 v_Normal;

uniform int u_Animate;
uniform vec3 u_BGColor;
uniform vec3 u_DotColor;
uniform vec3 u_CameraPosition;
uniform vec3 u_LightPosition;
uniform sampler2D u_Texture;


float Wave(float a, float f, float v)
{
	return a * sin(u_ElapsedTime + f * v * PI);
}

void main()
{
	vec3 normal = normalize(v_Normal);
	float Ar = u_Ad / 2.0;
	float Br = u_Bd / 2.0;

	float animation = u_Animate == 1 ? 1.0 : 0.0;
	vec2 st = v_TexCoord + vec2(Wave(0.5, 1.5, v_TexCoord.t), Wave(0.5, 1.5, v_TexCoord.s)) * animation;

	float s = st.x;
	float t = st.y;

	int cellX = int(s / u_Ad);
	int cellY = int(t / u_Bd);

	float sc = float(cellX) * u_Ad + Ar;
	float tc = float(cellY) * u_Bd + Br;

	float dX = ((s - sc) / Ar) * ((s - sc) / Ar);
	float dY = ((t - tc) / Br) * ((t - tc) / Br);

	float d = dX + dY;
	float pct = smoothstep(1.0 + u_Tol, 1.0 - u_Tol, d);

	vec3 color = mix(u_BGColor, u_DotColor, pct) * texture(u_Texture, v_TexCoord).rgb;

	vec3 lightDirection = normalize(u_LightPosition - v_WorldPosition);
	vec3 viewDirection = normalize(u_CameraPosition - v_WorldPosition);
	float diffuse = max(0.0, dot(normal, lightDirection));

	vec3 halfway = normalize(lightDirection + viewDirection);
	float specular = pow(max(dot(normal, halfway), 0.0), u_Shininess * u_Shininess);

	vec3 diffuseColor = color * diffuse;
	vec3 ambientColor = color * u_AmbientStrength;
	vec3 specularColor = specular * vec3(1.0);

	vec3 result = diffuseColor + ambientColor + specularColor;

	o_Color = vec4(result, 1.0);
}