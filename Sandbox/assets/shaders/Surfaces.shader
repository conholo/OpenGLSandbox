#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;


#define PI 3.14159265359

uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;
uniform float u_VertexCounter;
uniform int u_AnimateVertex;
uniform int u_SurfaceType;

out vec2 v_TexCoord;
out vec3 v_WorldPosition;
out vec3 v_Normal;

float Remap(float value, float from1, float to1, float from2, float to2)
{
	return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
}

vec3 RemapVector3(vec3 vec)
{
	float x = Remap(vec.x, -0.5f, 0.5f, -1.0f, 1.01f);
	float y = Remap(vec.y, -0.5f, 0.5f, -1.0f, 1.01f);
	float z = Remap(vec.z, -0.5f, 0.5f, -1.0f, 1.01f);

	return vec3(x, y, z);
}

float Wave(float x, float z, float t)
{
	return sin(PI * (x + z + t));
}

float MultiWave(float x, float z, float t)
{
	float y = sin(PI * (x + 0.5 * t));
	y += 0.5 * sin(2.0 * PI * (z + t));
	y += sin(PI * (x + z + 0.25 * t));

	return y * (1.0 / 2.5);
}

float Ripple(float x, float z, float t)
{
	float d = sqrt(x * x + z * z);
	float y = sin(5.0 * (PI * d - t));
	return y / (0.5 + 10.0 * d);
}

void main()
{
	vec3 position = u_AnimateVertex == 1 ? RemapVector3(a_Position) : a_Position;
	vec3 normal = a_Normal;

	if (u_AnimateVertex == 1)
	{
		position.z = position.y;
		if (u_SurfaceType == 1)
			position.y = Ripple(position.x, position.z, u_VertexCounter);
		else if (u_SurfaceType == 2)
			position.y = Wave(position.x, position.z, u_VertexCounter);
		else if (u_SurfaceType == 3)
			position.y = MultiWave(position.x, position.z, u_VertexCounter);

		normal.z = normal.y;
		if(u_SurfaceType == 1)
			normal.y = max(Ripple(normal.x, normal.z, u_VertexCounter), 0.001);
		else if (u_SurfaceType == 2)
			normal.y = max(Wave(normal.x, normal.z, u_VertexCounter), 0.001);
		else if (u_SurfaceType == 3)
			normal.y = max(MultiWave(normal.x, normal.z, u_VertexCounter), 0.001);
	}

	v_Normal = vec3(u_ModelMatrix * vec4(normal, 0.0));;
	v_WorldPosition = (u_ModelMatrix * vec4(position, 1.0)).xyz;
	v_TexCoord = a_TexCoord;

	gl_Position = u_MVP * vec4(position, 1.0);
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
in vec3 v_Normal;
in vec3 v_WorldPosition;

uniform float u_FragmentCounter;
uniform int u_AnimateFragment;

uniform vec3 u_AmbientColor;
uniform float u_AmbientStrength;

uniform vec3 u_DiffuseColor;
uniform float u_DiffuseStrength;

uniform vec3 u_SpecularColor;
uniform float u_SpecularStrength;

uniform float u_Shininess;

uniform vec3 u_CameraPosition;

uniform sampler2D u_Texture;
uniform Light u_Light;


#define OCTAVES 8

//https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
vec2 hash(vec2 p)
{
	p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
	return fract(sin(p) * 43758.5453);
}

float voronoi(in vec2 x)
{
	vec2 cellID = floor(x);
	vec2 fractPart = fract(x);

	float F1 = 1.0;
	float F2 = 1.0;

	for (int j = -1; j <= 1; j++)
	{
		for (int i = -1; i <= 1; i++)
		{
			vec2 offset = vec2(i, j);
			vec2 randomPoint = hash(cellID + offset);

			randomPoint = 0.5 + 0.41 * sin(u_FragmentCounter + 20.0 * randomPoint);
			vec2 final = offset - fractPart + randomPoint;

			float sqrDistance = dot(final, final);

			if (sqrDistance < F1)
			{
				F2 = F1;
				F1 = sqrDistance;
			}
		}
	}

	float c = F1;
	return max(c, 0.5);
}

float fbm(in vec2 position)
{
	float value = 0.0;
	float frequency = 0.0;
	float amplitude = 0.5;

	for (int i = 0; i < OCTAVES; i++)
	{
		value += amplitude * voronoi(position);
		frequency += amplitude;
		amplitude *= 0.5;
		position *= 2.0;
	}

	value /= frequency;
	return value;
}

vec3 BlinnPhong(Light light, vec3 normal)
{
	vec3 directionToLight = normalize(light.Position - v_WorldPosition);
	vec3 directionToViewer = normalize(u_CameraPosition - v_WorldPosition);

	float ndotl = max(dot(normal, directionToLight), 0);
	
	vec3 halfwayVector = normalize(directionToLight + directionToViewer);
	float ndoth = max(dot(normal, halfwayVector), 0);
	float specValue = pow(ndoth, u_Shininess * u_Shininess);

	vec3 ambient = u_AmbientColor * u_AmbientStrength;
	vec3 diffuse = ndotl * u_DiffuseColor * u_DiffuseStrength;
	vec3 specular = u_SpecularColor * specValue * u_SpecularStrength;

	return (ambient + diffuse + specular) * light.Color * light.Intensity;
}


void main()
{
	vec3 percent = vec3(v_TexCoord, 1.0f);
	vec3 normal = normalize(v_Normal);

	float worldDistance = length(v_WorldPosition);
	
	if (worldDistance > 100.0f)
		discard;

	vec3 samplerColor = texture(u_Texture, v_TexCoord).rgb;
	vec3 blinnPhong = BlinnPhong(u_Light, normal) * samplerColor;

	float animated = u_AnimateFragment == 1 ? 2.0 * fbm(v_TexCoord * 10.0) : 1.0;

	o_Color = vec4(blinnPhong * animated, 1.0);
}