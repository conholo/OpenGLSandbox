#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_WorldPosition;

void main()
{
	v_Normal = vec3(u_ModelMatrix * vec4(a_Normal, 1.0)).xyz;
	v_TexCoord = a_TexCoord;
	v_WorldPosition = vec3(u_ModelMatrix * vec4(a_Position, 1.0)).xyz;
	gl_Position = u_MVP * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

uniform vec3 u_Color = vec3(1.0, 1.0, 1.0);
uniform sampler2D u_Texture;

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_WorldPosition;

uniform vec3 u_CameraPosition;

uniform vec3 u_LightColor;
uniform vec3 u_LightPosition;
uniform float u_LightIntensity;

uniform vec3 u_AmbientColor;
uniform vec3 u_DiffuseColor;

uniform float u_DiffuseStrength;
uniform float u_AmbientStrength;
uniform float u_SpecularStrength;
uniform float u_Shininess;


vec3 CalculateLighting(vec3 normal)
{
	vec3 lightDirection = normalize(u_LightPosition - v_WorldPosition);
	vec3 viewDirection = normalize(u_CameraPosition - v_WorldPosition);

	// Ambient
	vec3 ambient = u_LightIntensity * mix(u_LightColor, u_AmbientColor, u_AmbientStrength);

	// Diffuse
	float NdotL = max(dot(normal, lightDirection), 0);
	vec3 diffuse = u_LightIntensity * NdotL * u_DiffuseStrength * u_LightColor;

	// Specular
	vec3 halfway = normalize(lightDirection + viewDirection);
	float specular = pow(max(dot(normal, halfway), 0.0), u_Shininess * u_Shininess);
	vec3 spec = u_LightIntensity * specular * u_LightColor * u_SpecularStrength;

	return (ambient + diffuse + spec) * u_DiffuseColor;
}

void main()
{
	vec3 normal = normalize(v_Normal);
	vec3 lighting = CalculateLighting(normal);

	vec3 color = texture(u_Texture, v_TexCoord).rgb * lighting;
	o_Color = vec4(color, 1.0);
}