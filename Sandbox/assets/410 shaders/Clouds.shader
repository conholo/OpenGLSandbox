#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_MVP;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;

out vec2 v_UV;
out vec3 v_WorldSpaceViewDirection;

void main()
{
	v_UV = a_TexCoord;
	vec3 viewVector = (u_InverseProjection * vec4(a_TexCoord * 2.0 - 1.0, 0.0, 1.0)).xyz;
	v_WorldSpaceViewDirection = (u_InverseView * vec4(viewVector, 0.0)).xyz;

	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct Sun
{
    vec3 LightPosition;
    float Intensity;
    vec3 LightColor;
};


uniform Sun u_Sun;

uniform vec3 u_SkyColorA;
uniform vec3 u_SkyColorB;

uniform float u_ElapsedTime;
uniform float u_TimeScale;
uniform bool u_Animate;
uniform float u_CloudOffsetScrollSpeed;
uniform float u_AnimationSpeed;

uniform sampler2D u_DepthTexture;
uniform sampler2D u_SceneTexture;
uniform sampler2D u_WeatherMap;
uniform sampler3D u_BaseShapeTexture;
uniform sampler3D u_DetailShapeTexture;

uniform float u_ContainerEdgeFadeDistance;
uniform float u_PhaseBlend;
uniform vec4 u_PhaseParams;
uniform float u_PowderConstant;
uniform float u_SilverLiningConstant;
uniform float u_DensityMultiplier;
uniform float u_DensityThreshold;
uniform float u_CloudScale;
uniform float u_DetailNoiseWeight;
uniform vec3 u_ShapeTextureOffset;
uniform vec4 u_ShapeNoiseWeights;
uniform vec3 u_DetailNoiseWeights;

uniform int u_LightSteps;
uniform int u_DensitySteps;
uniform vec3 u_BoundsMin;
uniform vec3 u_BoundsMax;
uniform vec3 u_WorldSpaceCameraPosition;

uniform float u_NearClip;
uniform float u_FarClip;

in vec2 v_UV;
in vec3 v_WorldSpaceViewDirection;

// Debug
uniform sampler2D u_DisplayTexture2D;
uniform sampler3D u_DisplayTexture3D;
uniform vec2 u_ScreenResolution;
uniform float u_PercentOfScreen;
uniform int u_DisplayIndex;

uniform bool u_ShowAllChannels;
uniform bool u_GreyScale;
uniform bool u_ShowAlpha;
uniform float u_DepthSlice;
uniform vec4 u_ChannelWeights;

#define PERLIN_DISPLAY 0
#define SHAPE_DISPLAY 1


float Remap(float v, float minOld, float maxOld, float minNew, float maxNew)
{
    return minNew + (v - minOld) * (maxNew - minNew) / (maxOld - minOld);
}


// Returns (dstToBox, dstInsideBox). If ray misses box, dstInsideBox will be zero
vec2 RayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayPosition, vec3 invRaydir)
{
    // Adapted from: http://jcgt.org/published/0007/03/04/
    vec3 t0 = (boundsMin - rayPosition) * invRaydir;
    vec3 t1 = (boundsMax - rayPosition) * invRaydir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);

    float dstA = max(max(tmin.x, tmin.y), tmin.z);
    float dstB = min(tmax.x, min(tmax.y, tmax.z));

    // CASE 1: ray intersects box from outside (0 <= dstA <= dstB)
    // dstA is dst to nearest intersection, dstB dst to far intersection

    // CASE 2: ray intersects box from inside (dstA < 0 < dstB)
    // dstA is the dst to intersection behind the ray, dstB is dst to forward intersection

    // CASE 3: ray misses box (dstA > dstB)

    float dstToBox = max(0, dstA);
    float dstInsideBox = max(0, dstB - dstToBox);
    return vec2(dstToBox, dstInsideBox);
}

float CalculateLinearDepth(float depth)
{
    float near = u_NearClip;
    float far = u_FarClip;

    float ndcDepth = depth * 2.0 - 1.0;
    float linearDepth = (2.0 * near * far) / (far + near - ndcDepth * (far - near));

    return linearDepth;
}

float SampleDensity(vec3 rayPosition)
{
    vec3 containerSize = u_BoundsMax - u_BoundsMin;
    vec3 containerCenter = (u_BoundsMax + u_BoundsMin) * 0.5;
    vec3 uvw = (containerSize * 0.5 + rayPosition) * (1.0 / containerSize) * u_CloudScale;
    float animationSpeed = u_Animate ? u_AnimationSpeed : 0.0;
    float time = u_TimeScale * u_ElapsedTime;
    vec3 animationOffset = vec3(time, time * 0.1, time * 0.2) * animationSpeed;
    vec3 shapeSamplePosition = uvw + u_ShapeTextureOffset * u_CloudOffsetScrollSpeed + animationOffset;

    float distanceFromEdgeX = min(u_ContainerEdgeFadeDistance, min(rayPosition.x - u_BoundsMin.x, u_BoundsMax.x - rayPosition.x));
    float distanceFromEdgeZ = min(u_ContainerEdgeFadeDistance, min(rayPosition.z - u_BoundsMin.z, u_BoundsMax.z - rayPosition.z));
    float edgeWeight = min(distanceFromEdgeX, distanceFromEdgeZ) / u_ContainerEdgeFadeDistance;

    vec2 weatherMapUV = (containerSize.xz * 0.5 + (rayPosition.xz - containerCenter.xz)) / max(containerSize.z, containerSize.x);
    float weatherMapValue = texture(u_WeatherMap, weatherMapUV).x;
    float gMin = Remap(weatherMapValue, 0.0, 1.0, 0.1, 0.5);
    float gMax = Remap(weatherMapValue, 0.0, 1.0, gMin, 0.9);
    float heightPercent = (rayPosition.y - u_BoundsMin.y) / containerSize.y;
    float heightGradient = Remap(heightPercent, 0.0, gMin, 0.0, 1.0) * Remap(heightPercent, 1.0, gMax, 0.0, 1.0);
    heightGradient *= edgeWeight;

    vec4 shape = texture(u_BaseShapeTexture, shapeSamplePosition);
    vec4 normalizedShapeWeights = u_ShapeNoiseWeights / dot(u_ShapeNoiseWeights, vec4(1.0));
    float shapeFBM = dot(shape, normalizedShapeWeights) * heightGradient;
    float baseShapeDensity = max(0.0, shapeFBM - u_DensityThreshold) * u_DensityMultiplier;

    if (baseShapeDensity <= 0.0)
        return 0.0;

    vec3 detailAnimationOffset = vec3(time, time * 0.15, time * 0.25) * animationSpeed;
    vec3 detailSamplePosition = uvw + animationOffset;
    vec3 detail = texture(u_DetailShapeTexture, detailSamplePosition).rgb;
    vec3 normalizedDetailWeights = u_DetailNoiseWeights / dot(u_DetailNoiseWeights, vec3(1.0));
    float detailFBM = dot(detail, normalizedDetailWeights) * edgeWeight * u_DetailNoiseWeight;

    return baseShapeDensity - detailFBM;
}


float HG(float a, float g)
{
    float g2 = g * g;
    return (1 - g2) / (4.0 * 3.1415 * pow(1.0 + g2 - 2.0 * g * a, 1.5));
}

float Beers(float density)
{
    return exp(-density);
}

float BeersPowder(float density)
{
    return 1.0 - exp(-density * 2.0);
}

float PhaseFn(float angle)
{
    float blend = u_PhaseBlend;
    float hgBlend = HG(angle, u_PhaseParams.x) * (1.0 - blend) + HG(angle, u_PhaseParams.y) * blend;
    return u_PhaseParams.z + hgBlend * u_PhaseParams.w;
}

float LightMarch(vec3 hitPoint)
{
    vec3 directionToLight = normalize(u_Sun.LightPosition - hitPoint);

    float distanceRemainingInVolume = RayBoxDst(u_BoundsMin, u_BoundsMax, hitPoint, 1.0 / directionToLight).y;

    float stepSize = distanceRemainingInVolume / float(u_LightSteps);

    hitPoint += directionToLight * stepSize * 0.5;

    float transmittance = 1.0;
    float totalDensity = 0.0f;

    for (int i = 0; i < u_LightSteps; i++)
    {
        float density = SampleDensity(hitPoint);
        totalDensity += max(0.0, density * stepSize);

        hitPoint += directionToLight * stepSize;
    }

    transmittance = Beers(totalDensity * u_SilverLiningConstant);

    return transmittance;
}

void main()
{
    vec4 color = vec4(0.0);

    float aspect = u_ScreenResolution.x / u_ScreenResolution.y;
    vec2 pctScreen = gl_FragCoord.xy / u_ScreenResolution;
	vec2 bottomLeftTexCoords = u_ScreenResolution - u_ScreenResolution * vec2(u_PercentOfScreen, aspect * u_PercentOfScreen);
    if (gl_FragCoord.x >= bottomLeftTexCoords.x && gl_FragCoord.y >= bottomLeftTexCoords.y)
    {
        vec2 percentCoord = gl_FragCoord.xy / u_ScreenResolution;
        vec2 minExtents = 1.0 - vec2(u_PercentOfScreen, aspect * u_PercentOfScreen);

        float texX = Remap(percentCoord.x, minExtents.x, 1.0, 0.0, 1.0);
        float texY = Remap(percentCoord.y, minExtents.y, 1.0, 0.0, 1.0);

        vec2 coord = vec2(texX, texY);

        switch (u_DisplayIndex)
        {
            case PERLIN_DISPLAY:
            {
                color = texture(u_DisplayTexture2D, coord);
                break;
            }
            case SHAPE_DISPLAY:
            {
                vec4 result = texture(u_DisplayTexture3D, vec3(coord, u_DepthSlice));

                if (u_ShowAllChannels)
                {
                    color = vec4(result.rgb, 1.0);
                }
                else
                {
                    vec4 channelMask = u_ChannelWeights * result;

                    if (u_ShowAlpha)
                    {
                        color = vec4(vec3(channelMask.a), 1.0);
                    }
                    else if (u_GreyScale)
                    {
                        float g = dot(channelMask, vec4(1.0));
                        color = vec4(vec3(g), 1.0);
                    }
                    else
                    {
                        color = vec4(channelMask.rgb, 1.0);
                    }
                }

                break;
            }
            default:
                break;
        }
    }
    else
    {
        vec3 rayPosition = u_WorldSpaceCameraPosition;
        vec3 rayDirection = normalize(v_WorldSpaceViewDirection);

        vec2 rayBoxInfo = RayBoxDst(u_BoundsMin, u_BoundsMax, rayPosition, 1.0f / rayDirection);
        float distanceToBox = rayBoxInfo.x;
        float distanceInsideBox = rayBoxInfo.y;

        float nonLinearDepth = texture(u_DepthTexture, v_UV).r;
        float linearDepth = CalculateLinearDepth(nonLinearDepth);

        vec3 intersectionPoint = rayPosition + rayDirection * distanceToBox;

        float distanceTravelled = 0.0;
        float distanceLimit = min(linearDepth - distanceToBox, distanceInsideBox);

        vec3 directionToLight = normalize(u_Sun.LightPosition - u_WorldSpaceCameraPosition);

        float cosLightViewer = dot(rayDirection, directionToLight);
        float phaseValue = PhaseFn(cosLightViewer);

        float stepSize = distanceLimit / float(u_DensitySteps);
        float totalDensity = 0.0;
        float transmittance = 1.0;
        vec3 lightEnergy = vec3(0.0);

        while (distanceTravelled < distanceLimit)
        {
            rayPosition = intersectionPoint + rayDirection * distanceTravelled;

            float density = SampleDensity(rayPosition);

            if (density > 0)
            {
                float lightTransmittance = LightMarch(rayPosition);
                lightEnergy += density * stepSize * transmittance * lightTransmittance * phaseValue;
                transmittance *= exp(-density * stepSize * u_PowderConstant);
                if (transmittance < 0.01)
                    break;
            }

            distanceTravelled += stepSize;
        }

        vec3 skyColor = mix(u_SkyColorA, u_SkyColorB, sqrt(abs(clamp(rayDirection.y, 0.0, 1.0))));
        vec3 background = texture(u_SceneTexture, v_UV).rgb;

        float focusedEye = pow(clamp(cosLightViewer, 0.0, 1.0), u_PhaseParams.x);
        float sunValue = clamp(HG(focusedEye, 0.9995), 0.0, 1.0) * transmittance;

        vec3 backgroundColor = background + skyColor;
        vec3 cloudColor = lightEnergy * u_Sun.LightColor;

        vec3 col = backgroundColor * transmittance + cloudColor;
        color = vec4(col * (1.0 - sunValue) + u_Sun.LightColor * sunValue, 1.0);
    }

    o_Color = vec4(color);
}