#include "Lambert.hlsli"

//struct VS_OUT
//{
//    float4 vertex : SV_POSITION;
//    float2 texcoord : TEXCOORD;
//    float3 normal : NORMAL;
//    float3 position : POSITION;
//    float3 tangent : TANGENT;
//    float4 shadow_position : TEXCOORD1;
//    float3 worldPos : TEXCOORD2;
//};

cbuffer CbDither : register(b6)
{
    float ditherAlpha;
    float3 ditherPadding;
};

Texture2D DiffuseMap : register(t0);
SamplerState LinearSampler : register(s0);

static const float bayer[4][4] =
{
    0.0 / 16.0, 8.0 / 16.0, 2.0 / 16.0, 10.0 / 16.0,
    12.0 / 16.0, 4.0 / 16.0, 14.0 / 16.0, 6.0 / 16.0,
     3.0 / 16.0, 11.0 / 16.0, 1.0 / 16.0, 9.0 / 16.0,
    15.0 / 16.0, 7.0 / 16.0, 13.0 / 16.0, 5.0 / 16.0
};

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = DiffuseMap.Sample(LinearSampler, pin.texcoord);

    int x = (int) pin.vertex.x % 4;
    int y = (int) pin.vertex.y % 4;
    float threshold = bayer[y][x];

    if (ditherAlpha < threshold)
        discard;

    return color;
}