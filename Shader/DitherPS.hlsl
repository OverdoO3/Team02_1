#include "Lambert.hlsli"

cbuffer CbDither : register(b6)
{
    float ditherAlpha;
    float3 ditherPadding;
};

Texture2D DiffuseMap : register(t0);
SamplerState LinearSampler : register(s0);

// 8x8 ディザ行列
static const float bayer8x8[8][8] =
{
    { 0, 48, 12, 60, 3, 51, 15, 63 },
    { 32, 16, 44, 28, 35, 19, 47, 31 },
    { 8, 56, 4, 52, 11, 59, 7, 55 },
    { 40, 24, 36, 20, 43, 27, 39, 23 },
    { 2, 50, 14, 62, 1, 49, 13, 61 },
    { 34, 18, 46, 30, 33, 17, 45, 29 },
    { 10, 58, 6, 54, 9, 57, 5, 53 },
    { 42, 26, 38, 22, 41, 25, 37, 21 }
};

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = DiffuseMap.Sample(LinearSampler, pin.texcoord);

    int px = ((int) pin.vertex.x) % 8;
    int py = ((int) pin.vertex.y) % 8;
    float threshold = bayer8x8[py][px] / 64.0;

    if (ditherAlpha < threshold)
        discard;
    
    color.a = 0.6f;

    return color;
}