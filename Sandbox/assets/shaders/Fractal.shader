#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

void main()
{
	gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

#define M_PI 3.14159265358979
#define MIN_DISTANCE 0.00001
#define MAX_DISTANCE 100.0
#define MAX_STEPS	 100

uniform float u_MengerScale;
uniform vec2 u_MousePosition;
uniform vec2 u_ScreenResolution;
uniform vec3 u_CameraPosition;
uniform vec3 u_LightPosition;
uniform float u_Elapsed;
uniform float u_DeltaTime;

mat3 SetCameraView(vec3 cameraPosition, vec3 target, float cr)
{
	vec3 cw = normalize(target - cameraPosition);
	vec3 cp = vec3(sin(cr), cos(cr), 0.0);
	vec3 cu = normalize(cross(cw, cp));
	vec3 cv = cross(cu, cw);
	return mat3(cu, cv, cw);
}

vec2 iBox(vec3 ro, vec3 rd, vec3 rad)
{
	vec3 m = 1.0 / rd;
	vec3 n = m * ro;
	vec3 k = abs(m) * rad;
	vec3 t1 = -n - k;
	vec3 t2 = -n + k;
	return vec2(max(max(t1.x, t1.y), t1.z),
		min(min(t2.x, t2.y), t2.z));
}

vec2 opU(vec2 d1, vec2 d2)
{
	return (d1.x < d2.x) ? d1 : d2;
}

float sdSphere(vec3 p, float s)
{
	return length(p) - s;
}

float sdBox(vec3 p, vec3 b)
{
	vec3 q = abs(p) - b;
	return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

vec3 MengerFold(in vec3 pos)
{
	vec3 z = pos;
	float a = min(z.x - z.y, 0.0);
	z.x -= a;
	z.y += a;
	a = min(z.x - z.z, 0.0);
	z.x -= a;
	z.z += a;
	a = min(z.y - z.z, 0.0);
	z.y -= a;
	z.z += a;

	return z;
}

vec3 MengerBox(in vec3 p)
{
	float d = sdBox(p - vec3(0.0, 2.0, 0.0), vec3(2.0));

	vec3 result = vec3(d, 1.0, 0.0);

	float scale = u_MengerScale;

	for (int m = 0; m < 3; m++)
	{
		vec3 a = mod(p * scale, 2.0) - 1.0;
		scale *= 3.0;
		vec3 r = abs(1.0 - 3.0 * abs(a));

		float da = max(r.x, r.y);
		float db = max(r.y, r.z);
		float dc = max(r.z, r.x);
		float c = (min(da, min(db, dc)) - 1.0) / scale;

		if (c > d)
		{
			d = c;
			result = vec3(d, 0.2 * da * db * dc, (1.0 + float(m)) / 4.0);
		}
	}

	result.x = min(result.x, p.y + 1.0);
	return result;
}



vec2 DE(vec3 position)
{
	vec3 pos = MengerFold(position);
	float sphere = sdSphere(position - vec3(0.0, 0.25, 0.0), 0.25);

	vec3 mengerSponge = MengerBox(pos);
	vec2 result = opU(vec2(1e10, 0.0), vec2(mengerSponge.x, 15.0));
	return result;
}


vec3 CalculateNormal(vec3 position)
{
	vec2 e = vec2(1.0, -1.0) * 0.5773 * 0.0005;
	return normalize(e.xyy * DE(position + e.xyy).x +
		e.yyx * DE(position + e.yyx).x +
		e.yxy * DE(position + e.yxy).x +
		e.xxx * DE(position + e.xxx).x);
}

vec2 Raycast(vec3 rayOrigin, vec3 rayDirection)
{
	vec2 result = vec2(-1.0);

	float tMin = 1.0;
	float tMax = 20.0;

	float tPointOne = -rayOrigin.y / rayDirection.y;

	if (tPointOne > 0.0)
	{
		tMax = min(tMax, tPointOne);
		result = vec2(tPointOne, 1.0);
	}
	
	vec2 box = iBox(rayOrigin - vec3(0.0, 0.4, -0.5), rayDirection, vec3(5.0, 5.0, 5.0));

	if (box.x < box.y && box.y > 0.0 && box.x < tMax)
	{
		tMin = max(box.x, tMin);
		tMax = min(box.y, tMax);

		float t = tMin;

		for (int i = 0; i < 70 && t < tMax; i++)
		{
			vec2 de = DE(rayOrigin + rayDirection * t);

			if (abs(de.x) < (0.0001 * t))
			{
				result = vec2(t, de.y);
				break;
			}

			t += de.x;
		}
	}

	return result;
}

float CheckersGradientBox(vec2 position, vec2 dpdx, vec2 dpdy)
{
	vec2 w = abs(dpdx) + abs(dpdy) + 0.001;
	vec2 i = 2.0 * (abs(fract((position - 0.5 * w) * 0.5) - 0.5) - abs(fract((position + 0.5 * w) * 0.5) - 0.5)) / w;
	return 0.5 - 0.5 * i.x * i.y;
}

float CalculateSoftShadows(vec3 rayOrigin, vec3 rayDirection, float mint, float tMax)
{
	float tp = (0.8 - rayOrigin.y) / rayDirection.y;
	if (tp > 0.0)
		tMax = min(tMax, tp);

	float result = 1.0;
	float t = mint;

	for (int i = 0; i < 24; i++)
	{
		float de = DE(rayOrigin + rayDirection * t).x;
		float s = clamp(8.0 * de / t, 0.0, 1.0);
		result = min(result, s * s * (3.0 - 2.0 * s));
		t += clamp(de, 0.02, 0.2);

		if (result < 0.004 || t > tMax) break;
	}

	return clamp(result, 0.0, 1.0);
}

float CalculateAmbientOcclusion(vec3 position, vec3 normal)
{
	float occlusion = 0.0;
	float scale = 1.0;
	for (int i = 0; i < 5; i++)
	{
		float h = 0.01 + 0.12 * float(i) / 4.0;
		float d = DE(position + h * normal).x;
		occlusion += (h - d) * scale;
		scale *= 0.95;
		if (occlusion > 0.35) break;
	}

	return clamp(1.0 - 3.0 * occlusion, 0.0, 1.0) * (0.5 + 0.5 * normal.y);
}

void main()
{
	vec2 uv = (2.0 * gl_FragCoord.xy - u_ScreenResolution) / u_ScreenResolution.y;
	float time = u_Elapsed * 1.5;
	vec2 mouse = u_MousePosition / u_ScreenResolution;

	// Set Camera
	vec3 target = vec3(0.0f, 0.0f, 0.0f);
	vec3 cameraPosition = target + vec3(12.0 * cos(0.1 * time + mouse.x * 5.0), 5.0 + mouse.y * 2.0, 12.0 * sin(0.1 * time + mouse.x * 5.0));
	mat3 cameraView = SetCameraView(cameraPosition, target, 0.0);
	const float focalLength = 2.5;

	vec3 rayDirection = cameraView * normalize(vec3(uv, focalLength));

	vec2 pointX = (2.0 * (gl_FragCoord.xy + vec2(1.0, 0.0)) - u_ScreenResolution) / u_ScreenResolution.y;
	vec2 pointY = (2.0 * (gl_FragCoord.xy + vec2(0.0, 1.0)) - u_ScreenResolution) / u_ScreenResolution.y;
	vec3 rayDx = cameraView * normalize(vec3(pointX, focalLength));
	vec3 rayDy = cameraView * normalize(vec3(pointY, focalLength));
	
	vec2 result = Raycast(cameraPosition, rayDirection);

	vec3 color = vec3(0.7, 0.7, 0.9) - max(rayDirection.y, 0.0) * 0.3;

	float dist = result.x;
	float height = result.y;

	if (height > -0.5)
	{
		vec3 position = cameraPosition + rayDirection * dist;
		vec3 normal = (height < 1.5) ? vec3(0.0, 1.0, 0.0) : CalculateNormal(position);

		vec3 reflection = reflect(rayDirection, normal);

		color = 0.2 + 0.2 * sin(height * 2.0 + vec3(0.0, 1.0, 2.0));
		float ks = 1.0;

		if (height < 1.5)
		{
			vec3 dpdx = cameraPosition.y * (rayDirection / rayDirection.y - rayDx / rayDx.y);
			vec3 dpdy = cameraPosition.y * (rayDirection / rayDirection.y - rayDy / rayDy.y);
			float checkers = CheckersGradientBox(4.0 * position.xz, 4.0 * dpdx.xz, 4.0 * dpdy.xz);
			color = 0.15 * checkers * vec3(0.05);
			ks = 0.6;
		}

		float occlusion = CalculateAmbientOcclusion(position, normal);
		vec3 lightIntensity = vec3(0.0);

		// Sun
		{
			vec3 light = normalize(vec3(-0.5, 0.4, -0.6));
			vec3 halfVector = normalize(light - rayDirection);
			float diffuse = clamp(dot(normal, light), 0.0, 1.0);
			diffuse *= CalculateSoftShadows(position, light, 0.02, 2.5);
			float specular = pow(clamp(dot(normal, halfVector), 0.0, 1.0), 16.0);
			specular *= diffuse;
			specular *= 0.04 + 0.96 * pow(clamp(1.0 - dot(halfVector, light), 0.0, 1.0), 5.0);
			lightIntensity += color * 2.20 * diffuse * vec3(1.30, 1.00, 0.70);
			lightIntensity += 5.0 * specular * vec3(1.30, 1.00, 0.70) * ks;
		}

		// Sky
		{
			float diffuse = sqrt(clamp(0.5 + 0.5 * normal.y, 0.0, 1.0));
			diffuse *= occlusion;
			float specular = smoothstep(-0.2, 0.2, reflection.y);
			specular *= diffuse;
			specular *= 0.04 + 0.96 * pow(clamp(1.0 + dot(normal, rayDirection), 0.0, 1.0), 5.0);
			specular *= CalculateSoftShadows(position, reflection, 0.02, 2.5);
			lightIntensity += color * 0.60 * diffuse * vec3(0.40, 0.60, 1.15);
			lightIntensity += 2.0 * specular * vec3(0.40, 0.60, 1.30) * ks;
		}
		
		// Back
		{
			float diffuse = max(dot(normal, normalize(vec3(0.5, 0.0, 0.6))), 0.0) * clamp(1.0 - position.y, 0.0, 1.0);
			diffuse *= occlusion;
			lightIntensity += color * 0.55 * diffuse * vec3(0.25, 0.25, 0.25);
		}

		// sss
		{
			float diffuse = pow(clamp(1.0 + dot(normal, rayDirection), 0.0, 1.0), 2.0);
			diffuse *= occlusion;
			lightIntensity += color * 0.25 * diffuse * vec3(1.00, 1.00, 1.00);
		}

		color = lightIntensity;
		color = mix(color, vec3(0.7, 0.7, 0.9), 1.0 - exp(-0.0001 * dist * dist * dist));
		color = clamp(color, 0.0, 1.0);
	}

	color = pow(color, vec3(0.4545));

	o_Color = vec4(color, 1.0);
}