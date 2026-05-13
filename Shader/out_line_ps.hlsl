#include "Lambert.hlsli"

cbuffer OutlineColor : register(b7)
{
    float4 outlineColor;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    return outlineColor;
}