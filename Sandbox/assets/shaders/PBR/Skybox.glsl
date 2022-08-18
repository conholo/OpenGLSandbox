#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 0) out vec3 v_Position;

uniform mat4 u_ViewProjection;

void main()
{
	vec4 position = u_ViewProjection * vec4(a_Position, 1.0);
	gl_Position = position.xyww;

	v_Position = vec3(a_Position.x, a_Position.y, -a_Position.z);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_FilteredEnvironmentColor;
layout(location = 1) out vec4 o_UnfilteredEnvironmentColor;
layout(location = 2) out vec4 o_IrradianceEnvironmentColor;
layout(location = 0) in vec3 v_Position;

uniform samplerCube u_FilteredRadianceMap;
uniform samplerCube u_UnfilteredRadianceMap;
uniform samplerCube u_IrradianceMap;

/* 
    x - u_FilteredRadianceMap, 
    y - u_UnfilteredRadianceMap, 
    z - u_IrradianceMap
*/
uniform vec3 u_LODs;
uniform vec3 u_Intensities;

void main()
{
	o_FilteredEnvironmentColor = textureLod(u_FilteredRadianceMap, v_Position, u_LODs.x) * u_Intensities.x;
	o_FilteredEnvironmentColor.a = 1.0;
	
    o_UnfilteredEnvironmentColor = textureLod(u_UnfilteredRadianceMap, v_Position, u_LODs.y) * u_Intensities.y;
	o_UnfilteredEnvironmentColor.a = 1.0;

    o_IrradianceEnvironmentColor = textureLod(u_IrradianceMap, v_Position, u_LODs.z) * u_Intensities.z;
	o_IrradianceEnvironmentColor.a = 1.0;
}
