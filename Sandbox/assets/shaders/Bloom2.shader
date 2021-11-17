#type compute
#version 450 core

layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

const float Epsilon = 1.0e-4;

uniform sampler2D u_Texture;
uniform sampler2D u_BloomTexture;

uniform vec4 u_Params; // (x) threshold, (y) threshold - knee, (z) knee * 2, (w) 0.25 / knee
uniform float u_LOD;
uniform int u_Mode; // See defines below

#define MODE_PREFILTER      0
#define MODE_DOWNSAMPLE     1
#define MODE_UPSAMPLE_FIRST 2
#define MODE_UPSAMPLE       3

// [Jimenez14] http://goo.gl/eomGso
// . . . . . . .
// . A . B . C .
// . . D . E . . 
// . F . G . H .
// . . I . J . .
// . K . L . M .
// . . . . . . .

vec3 DownsampleBox13(sampler2D tex, float lod, vec2 uv, vec2 texelSize)
{
    vec3 A = textureLod(tex, uv + texelSize * vec2(-2.0, -2.0), lod).rgb;
    vec3 B = textureLod(tex, uv + texelSize * vec2( 0.0, -2.0), lod).rgb;
    vec3 C = textureLod(tex, uv + texelSize * vec2( 2.0, -2.0), lod).rgb;

    vec3 D = textureLod(tex, uv + texelSize * vec2(-1.0, -1.0), lod).rgb;
    vec3 E = textureLod(tex, uv + texelSize * vec2( 1.0, -1.0), lod).rgb;

    vec3 F = textureLod(tex, uv + texelSize * vec2(-2.0,  0.0), lod).rgb;
    vec3 G = textureLod(tex, uv                               , lod).rgb;
    vec3 H = textureLod(tex, uv + texelSize * vec2( 2.0,  0.0), lod).rgb;

    vec3 I = textureLod(tex, uv + texelSize * vec2(-1.0,  1.0), lod).rgb;
    vec3 J = textureLod(tex, uv + texelSize * vec2( 1.0,  1.0), lod).rgb;

    vec3 K = textureLod(tex, uv + texelSize * vec2(-2.0,  2.0), lod).rgb;
    vec3 L = textureLod(tex, uv + texelSize * vec2( 0.0,  2.0), lod).rgb;
    vec3 M = textureLod(tex, uv + texelSize * vec2( 2.0,  2.0), lod).rgb;

    vec2 weight = (1.0 / 4.0) * vec2(0.5, 0.125);

    // Center
    vec3 result = (D + E + I + J) * weight.x;
    // Top Left
    result += (A + B + G + F) * weight.y;
    // Top Right
    result += (B + C + G + H) * weight.y;
    // Bottom Left
    result += (F + G + K + L) * weight.y;
    // Bottom Right
    result += (G + H + M + L) * weight.y;


    return result;
}

// Quadratic color thresholding
// curve = (threshold - knee, knee * 2, 0.25 / knee)
vec4 QuadraticThreshold(vec4 color, float threshold, vec3 curve)
{
    // Maximum pixel brightness
    float brightness = max(max(color.r, color.g), color.b);
    // Quadratic curve
    float rq = clamp(brightness - curve.x, 0.0, curve.y);
    rq = (rq * rq) * curve.z;
    color *= max(rq, brightness - threshold) / max(brightness, Epsilon);
    return color;
}

vec4 Prefilter(vec4 color)
{
    float clampValue = 20.0f;
    color = min(vec4(clampValue), color);
    color = QuadraticThreshold(color, u_Params.x, u_Params.yzw);
    return color;
}

vec3 UpsampleTent9(sampler2D tex, float lod, vec2 uv, vec2 texelSize, float sampleScale)
{
    vec4 offset = texelSize.xyxy * vec4(1.0f, 1.0f, -1.0f, 0.0f) * sampleScale;

    vec3 result = vec3(0.0f);


    result += textureLod(tex, uv - offset.xy, lod).rgb;
    result += textureLod(tex, uv - offset.wy, lod).rgb * 2.0;
    result += textureLod(tex, uv - offset.zy, lod).rgb;

    result += textureLod(tex, uv + offset.zw, lod).rgb * 2.0;
    result += textureLod(tex, uv,             lod).rgb * 1.0f;
    result += textureLod(tex, uv + offset.xw, lod).rgb * 2.0;

    result += textureLod(tex, uv + offset.zy, lod).rgb;
    result += textureLod(tex, uv + offset.wy, lod).rgb * 2.0;
    result += textureLod(tex, uv + offset.xy, lod).rgb;

    return result * (1.0f / 16.0f);
}

layout(local_size_x = 4, local_size_y = 4) in;
void main()
{
    vec2 imgSize = vec2(imageSize(o_Image));

    ivec2 invocID = ivec2(gl_GlobalInvocationID);
    vec2 texCoords = vec2(float(invocID.x) / imgSize.x, float(invocID.y) / imgSize.y);
    texCoords += (1.0f / imgSize) * 0.5f;

    vec2 texSize = vec2(textureSize(u_Texture, int(u_LOD)));
    vec4 color = vec4(1, 1, 0, 1);
    if (u_Mode == MODE_PREFILTER)
    {
        color.rgb = DownsampleBox13(u_Texture, 0, texCoords, 1.0f / texSize);
        color = Prefilter(color);
        color.a = 1.0f;
    }
    else if (u_Mode == MODE_UPSAMPLE_FIRST)
    {
        vec2 bloomTexSize = vec2(textureSize(u_Texture, int(u_LOD + 1.0f)));
        float sampleScale = 1.0f;
        vec3 upsampledTexture = UpsampleTent9(u_Texture, u_LOD + 1.0f, texCoords, 1.0f / bloomTexSize, sampleScale);

        vec3 existing = textureLod(u_Texture, texCoords, u_LOD).rgb;
        color.rgb = existing + upsampledTexture;
    }
    else if (u_Mode == MODE_UPSAMPLE)
    {
        vec2 bloomTexSize = vec2(textureSize(u_BloomTexture, int(u_LOD + 1.0f)));
        float sampleScale = 1.0f;
        vec3 upsampledTexture = UpsampleTent9(u_BloomTexture, u_LOD + 1.0f, texCoords, 1.0f / bloomTexSize, sampleScale);

        vec3 existing = textureLod(u_Texture, texCoords, u_LOD).rgb;
        color.rgb = existing + upsampledTexture;
    }
    else if (u_Mode == MODE_DOWNSAMPLE)
    {
        color.rgb = DownsampleBox13(u_Texture, u_LOD, texCoords, 1.0f / texSize);
    }

    imageStore(o_Image, ivec2(gl_GlobalInvocationID), color);
}
