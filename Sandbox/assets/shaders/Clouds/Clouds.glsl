#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_Tangent;
layout(location = 3) in vec2 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 u_MVP;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;

out vec2 v_UV;
out vec3 v_WorldSpaceViewDirection;

void main()
{
	v_UV = a_TexCoord;
    // Put the coordidnate into view space.  Keep w as 1.0 to maintain z information.
	vec3 viewVector = (u_InverseProjection * vec4(a_TexCoord * 2.0 - 1.0, 0.0, 1.0)).xyz;
    // Put the coordinate into world space, but convert it to a direction.
    // We don't want the 4th columns data from the view matrix - as that would give us a point.
	v_WorldSpaceViewDirection = (u_InverseView * vec4(viewVector, 0.0)).xyz;

    // Draw fullscreen quad.
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



// Sky/Sun Uniforms
uniform vec3 u_SkyColorA;
uniform vec3 u_SkyColorB;
uniform Sun u_Sun;
uniform float u_SunSize;

// Animation Uniforms
uniform float u_ElapsedTime;
uniform float u_TimeScale;
uniform bool u_Animate;
uniform float u_CloudOffsetScrollSpeed;
uniform float u_AnimationSpeed;

// Textures
uniform sampler2D u_DepthTexture;
uniform sampler2D u_SceneTexture;
uniform sampler2D u_CurlNoise;
uniform sampler2D u_WeatherMap;
uniform sampler2D u_BlueNoiseTexture;
uniform sampler3D u_BaseShapeTexture;
uniform sampler3D u_DetailShapeTexture;
uniform sampler2D u_WaterTexture;

// Phase Uniforms
uniform float u_PhaseBlend;
uniform vec4 u_PhaseParams;

uniform float u_PowderConstant;
uniform float u_SilverLiningConstant;


// Density Uniforms
uniform float u_DensityMultiplier;
uniform float u_DensityThreshold;
uniform float u_ExtinctionFactor;

uniform vec4 u_ShapeNoiseWeights;
uniform vec3 u_ShapeTextureOffset;

uniform vec3 u_CloudTypeWeights;
uniform float u_CloudTypeWeightStrength;

uniform vec3 u_DetailNoiseWeights;
uniform float u_DetailNoiseWeight;

uniform float u_CurlIntensity;

// Marching Uniforms
uniform int u_LightSteps;
uniform int u_DensitySteps;
uniform float u_RandomOffsetStrength;

// Bounds/Container Uniforms
uniform vec3 u_BoundsMin;
uniform vec3 u_BoundsMax;
uniform float u_CloudScale;
uniform float u_CloudScaleFactor;
uniform float u_ContainerEdgeFadeDistance;

// Camera Uniforms
uniform vec3 u_WorldSpaceCameraPosition;
uniform float u_NearClip;
uniform float u_FarClip;

in vec2 v_UV;
in vec3 v_WorldSpaceViewDirection;

// Debug
uniform bool u_DrawClouds;
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

#define DISPLAY_2D_TEXTURE 0
#define DISPLAY_3D_TEXTURE 1
#define PI 3.14159


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

float CubicPulse(float t, float c, float w)
{
    t = abs(t - c);
    if (t > w) return 0.0;
    t /= w;
    return 1.0 - t * t * (3.0 - 2.0 * t);
}

float Stratus(float t)
{
    float c = 0.125;
    float w = 0.040;
    return CubicPulse(t, c, w);
}

float Cumulus(float t)
{
    float c = 0.265;
    float w = 0.200;
    return CubicPulse(t, c, w);
}

float Cumulonimbus(float t)
{
    float c = .55;
    float w = .52;
    return CubicPulse(t, c, w);
}

float GetHeightFractionForPoint(float rayPositionHeight, float containerSizeHeight)
{
    return (rayPositionHeight - u_BoundsMin.y) / containerSizeHeight;
}

float GetHeightGradientFromPoint(float rayPositionHeight, float containerSizeHeight)
{
    float heightFraction = GetHeightFractionForPoint(rayPositionHeight, containerSizeHeight);
    vec3 heightMapping = vec3(Stratus(heightFraction), Cumulus(heightFraction), Cumulonimbus(heightFraction));
    vec3 normalizedCloudTypeWeights = u_CloudTypeWeights / dot(u_CloudTypeWeights, vec3(1.0));
    float heightValue = dot(heightMapping, normalizedCloudTypeWeights);

    return heightValue * u_CloudTypeWeightStrength;
}

vec2 SSUV(vec2 uv)
{
    float width = u_ScreenResolution.x;
    float height = u_ScreenResolution.y;
    const float scale = 1000;
    float x = uv.x * width;
    float y = uv.y * height;
    return vec2(x / scale, y / scale);
}

float SampleDensity(vec3 rayPosition)
{
    vec3 containerSize = u_BoundsMax - u_BoundsMin;
    vec3 containerCenter = (u_BoundsMax + u_BoundsMin) * 0.5;
    vec3 uvw = (containerSize * 0.5 + rayPosition) * (1.0 / u_CloudScaleFactor) * u_CloudScale;
    float animationSpeed = u_Animate ? u_AnimationSpeed : 0.0;
    float time = u_TimeScale * u_ElapsedTime;
    vec3 animationOffset = vec3(time, time * 0.1, time * 0.2) * animationSpeed;
    vec3 shapeSamplePosition = uvw + u_ShapeTextureOffset * u_CloudOffsetScrollSpeed + animationOffset;

    float distanceFromEdgeX = min(u_ContainerEdgeFadeDistance, min(rayPosition.x - u_BoundsMin.x, u_BoundsMax.x - rayPosition.x));
    float distanceFromEdgeZ = min(u_ContainerEdgeFadeDistance, min(rayPosition.z - u_BoundsMin.z, u_BoundsMax.z - rayPosition.z));
    float edgeWeight = min(distanceFromEdgeX, distanceFromEdgeZ) / u_ContainerEdgeFadeDistance;

    float heightGradient = GetHeightGradientFromPoint(rayPosition.y, containerSize.y);
    heightGradient *= edgeWeight;

    vec4 shape = texture(u_BaseShapeTexture, shapeSamplePosition);
    vec4 normalizedShapeWeights = u_ShapeNoiseWeights / dot(u_ShapeNoiseWeights, vec4(1.0));
    float shapeFBM = dot(shape, normalizedShapeWeights) * heightGradient;

    if (shapeFBM <= 0.0)
        return 0.0;

    vec2 containerSampleUV = (containerSize.xz * 0.5 + (rayPosition.xz - containerCenter.xz)) / max(containerSize.z, containerSize.x);
    rayPosition.xy += vec2(texture(u_CurlNoise, containerSampleUV)) * u_CurlIntensity;
    uvw = (containerSize * 0.5 + rayPosition) * (1.0 / u_CloudScaleFactor) * u_CloudScale;

    vec3 detailAnimationOffset = vec3(time * 0.05, time * 0.15, time * 0.25) * animationSpeed;
    vec3 detailSamplePosition = uvw + detailAnimationOffset;

    vec3 detail = texture(u_DetailShapeTexture, detailSamplePosition).rgb * u_DetailNoiseWeight;
    vec3 normalizedDetailWeights = u_DetailNoiseWeights / dot(u_DetailNoiseWeights, vec3(1.0));
    float heightFraction = GetHeightFractionForPoint(rayPosition.y, containerSize.y);
    float detailFBM = dot(detail, normalizedDetailWeights) * (1.0 - heightFraction + 0.3f);
    float finalCloud = shapeFBM - detailFBM;

    return clamp(max(0.0, finalCloud * u_DensityMultiplier - u_DensityThreshold), 0.0, 1.0);
}

float HG(float a, float g)
{
    float g2 = g * g;
    return ((1.0 - g2) / pow(1.0 + g2 - 2.0 * g * a, 3.0 / 2.0)) / 4.0 * PI;
}

float Beers(float density)
{
    return exp(-density);
}

float Powder(float density)
{
    return 1.0 - exp(-density * 2.0);
}

float PhaseFn(float angle)
{
    float blend = u_PhaseBlend;
    float hgBlend = HG(angle, u_PhaseParams.x) * (1.0 - blend) + HG(angle, u_PhaseParams.y) * blend;
    return u_PhaseParams.z + hgBlend * u_PhaseParams.w;
}

float LightTransmittanceAtDensity(float d, float phase)
{
    return Powder(d * u_PowderConstant) * Beers(d) * 2.0 * phase * u_SilverLiningConstant;
}

float LightMarch(vec3 hitPoint)
{
    vec3 directionToLight = normalize(u_Sun.LightPosition - hitPoint);
    float distanceRemainingInVolume = RayBoxDst(u_BoundsMin, u_BoundsMax, hitPoint, 1.0 / directionToLight).y;

    float stepSize = distanceRemainingInVolume / float(u_LightSteps);

    float transmittance = 1.0;
    float totalDensity = 0.0f;

    for (int i = 0; i < u_LightSteps; i++)
    {
        float density = SampleDensity(hitPoint);
        totalDensity += max(0.0, density * stepSize);

        hitPoint += directionToLight * stepSize;
    }

    vec3 inViewVector = normalize(hitPoint - u_WorldSpaceCameraPosition);
    float cosAngle = dot(directionToLight, inViewVector);
    transmittance = LightTransmittanceAtDensity(totalDensity, PhaseFn(cosAngle));

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
            case DISPLAY_2D_TEXTURE:
            {
                color = texture(u_DisplayTexture2D, coord);
                break;
            }
            case DISPLAY_3D_TEXTURE:
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
        vec3 directionToLight = normalize(u_Sun.LightPosition - u_WorldSpaceCameraPosition);

        float transmittance = 1.0;
        vec3 cloudColor = vec3(0.0);
        float nonLinearDepth = texture(u_DepthTexture, v_UV).r;
        float linearDepth = CalculateLinearDepth(nonLinearDepth);

        if (u_DrawClouds)
        {
            vec2 rayBoxInfo = RayBoxDst(u_BoundsMin, u_BoundsMax, rayPosition, 1.0f / rayDirection);
            float distanceToBox = rayBoxInfo.x;
            float distanceInsideBox = rayBoxInfo.y;

            vec3 intersectionPoint = rayPosition + rayDirection * distanceToBox;
            float randomRayOffset = texture(u_BlueNoiseTexture, SSUV(v_UV)).r;

            float distanceTraveled = max(0.0, randomRayOffset * u_RandomOffsetStrength);
            float distanceLimit = min(linearDepth - distanceToBox, distanceInsideBox);

            float stepSize = distanceLimit / float(u_DensitySteps);
            float totalDensity = 0.0;
            vec3 lightEnergy = vec3(0.0);

            while (distanceTraveled < distanceLimit)
            {
                rayPosition = intersectionPoint + rayDirection * distanceTraveled;
                float density = SampleDensity(rayPosition);

                if (density > 0.0)
                {
                    float lightTransmittance = LightMarch(rayPosition);
                    lightEnergy += density * stepSize * lightTransmittance;
                    transmittance *= Beers(density * stepSize * u_ExtinctionFactor);
                    if (transmittance < 0.01)
                        break;
                }

                distanceTraveled += stepSize;
            }

            cloudColor = lightEnergy * u_Sun.LightColor;
        }

        // 0 when linearDepth is 0 at camera frustrum base.
        
#if 1
        float skyContribution = step(u_FarClip, linearDepth);
        vec3 skyWater = texture(u_WaterTexture, v_UV).rgb * skyContribution;
        vec3 sceneColor = texture(u_SceneTexture, v_UV).rgb;
        float focusedEye = pow(clamp(dot(rayDirection, directionToLight), 0.0, 1.0), u_SunSize);
        float sunValue = clamp(HG(focusedEye, 0.9995), 0.0, 1.0) * transmittance;
        vec3 bgColor = sceneColor + skyWater;
        color = vec4(bgColor * transmittance + cloudColor + u_Sun.LightColor * sunValue, 1.0);
#else
        float dstFog = 1.0 - exp(-max(0, linearDepth) * 10 * .0001);
        vec3 skyColor = mix(u_SkyColorA, u_SkyColorB, sqrt(abs(clamp(rayDirection.y, 0.0, 1.0))));
        skyColor = dstFog * skyColor;
        vec3 sceneColor = texture(u_SceneTexture, v_UV).rgb;

        float focusedEye = pow(clamp(dot(rayDirection, directionToLight), 0.0, 1.0), u_PhaseParams.x);
        float sunValue = clamp(HG(focusedEye, 0.9995), 0.0, 1.0) * transmittance;

        vec3 backgroundColor = sceneColor * (1 - dstFog) + skyColor;

        vec3 col = backgroundColor * transmittance + cloudColor;
        color = vec4(col * (1.0 - sunValue) + u_Sun.LightColor * sunValue, 1.0);
#endif
    }

    o_Color = vec4(color);
}