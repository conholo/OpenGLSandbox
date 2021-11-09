#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_MVP;
uniform mat4 u_NormalMatrix;
uniform mat4 u_ModelViewMatrix;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_ViewPosition;

void main()
{
	v_TexCoord = a_TexCoord;
	v_Normal = vec3(u_NormalMatrix * vec4(a_Normal, 0.0));
	v_ViewPosition = vec3(u_ModelViewMatrix * vec4(a_Position, 1.0));

	gl_Position = u_MVP * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

uniform vec3 u_LightPosition;
uniform vec3 u_SpecularColor;
uniform vec3 u_Color;

uniform float u_AmbientStrength;
uniform float u_DiffuseStrength;
uniform float u_SpecularStrength;
uniform float u_Shininess;

in vec3 v_ViewPosition;
in vec3 v_Normal;
in vec2 v_TexCoord;

vec3 Phong(vec3 normal)
{
	vec3 lightDirection = normalize(u_LightPosition - v_ViewPosition);
	vec3 viewDirection = normalize(-v_ViewPosition);

	// Ambient
	vec3 ambient = u_Color * u_AmbientStrength;

	// Diffuse
	float NdotL = max(dot(normal, lightDirection), 0);
	vec3 diffuse = NdotL * u_DiffuseStrength * u_Color;

	// Specular
	vec3 halfway = normalize(viewDirection + lightDirection);
	float specular = pow(max(dot(normal, halfway), 0.0), u_Shininess * u_Shininess);
	vec3 spec = specular * u_SpecularColor * u_SpecularStrength;

	return (ambient + diffuse + spec);
}

void main()
{
	vec3 normal = normalize(v_Normal);

	vec3 result = Phong(normal);

	o_Color = vec4(result, 1.0);
}