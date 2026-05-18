#include "Scene.hlsli"

struct VS_OUT
{
    float4 vertex : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 position : POSITION;
    float3 tangent : TANGENT;
    float4 shadow_position : TEXCOORD1;
    float3 worldPos : TEXCOORD2;
};
