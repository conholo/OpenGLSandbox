#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_MVP;

out vec2 v_TexCoord;
out vec3 v_Normal;

void main()
{
	v_TexCoord = a_TexCoord;
	v_Normal = a_Normal;
	gl_Position = u_MVP * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

uniform vec3 u_Color;
uniform vec2 u_Jump;
uniform float u_Offset;
uniform float u_Tiling;
uniform float u_Time;
uniform float u_Speed;
uniform float u_FlowStrength;

uniform sampler2D u_NoiseTexture;
uniform sampler2D u_PortalTexture;
uniform sampler2D u_FlowMapTexture;
uniform sampler2D u_WaterNormalTexture;

in vec2 v_TexCoord;
in vec3 v_Normal;

vec3 FlowUV(vec2 uv, vec2 flowVector, vec2 jump, float offset, float tiling, float time, bool flowB)
{
	float phaseOffset = flowB ? 0.5 : 0;
	float progress = fract(time + phaseOffset);
	vec3 uvw;
	uvw.xy = uv - flowVector * (progress + offset);
	uvw.xy *= tiling;
	uvw.xy += phaseOffset;
	uvw.z = 1 - abs(1 - 2 * progress);
	return uvw;
}

void main()
{
	vec2 coord = v_TexCoord;

	vec3 normal = texture(u_WaterNormalTexture, coord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	vec2 flowVector = texture(u_NoiseTexture, coord).rg * 2 - 1;
	flowVector *= u_FlowStrength;

	float noise = texture(u_FlowMapTexture, coord).a;
	float time = u_Time * u_Speed + noise;

	vec3 uvwA = FlowUV(coord, flowVector, u_Jump, u_Offset, u_Tiling, time, false);
	vec3 uvwB = FlowUV(coord, flowVector, u_Jump, u_Offset, u_Tiling, time, true);

	vec3 portalColorA = vec3(texture(u_PortalTexture, uvwA.xy)) * uvwA.z;
	vec3 portalColorB = vec3(texture(u_PortalTexture, uvwB.xy)) * uvwB.z;

	o_Color = vec4(vec3(portalColorA.r + portalColorB.r) * u_Color, 1.0);
}