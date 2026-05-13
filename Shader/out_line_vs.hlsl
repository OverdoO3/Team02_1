#include "Lambert.hlsli"
#include "Skinning.hlsli"

cbuffer OutLineBuffer : register(b6)
{
    float outlineThickness;
    float3 outlinePadding;
}

VS_OUT main(
    float4 position : POSITION,
    float3 normal : NORMAL,
    float3 tangent : TANGENT,
    float2 texcoord : TEXCOORD,
    float4 color : COLOR,
    float4 boneWeights : BONE_WEIGHTS,
    uint4 boneIndices : BONE_INDICES)
{
    VS_OUT vout = (VS_OUT) 0;
    float4 skinned_position = SkinningPosition(position, boneWeights, boneIndices);
    float3 skinned_normal = SkinningVector(normal, boneWeights, boneIndices);
    
    //ñ@ê¸ï˚å¸Ç…ñcÇÁÇ‹ÇπÇÈ
    skinned_position.xyz += normalize(skinned_normal) * outlineThickness;
    
    vout.vertex = mul(skinned_position, viewProjection);
    vout.texcoord = texcoord;
    vout.normal = skinned_normal;
    vout.worldPos = skinned_position.xyz;
    return vout;
}