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
    uint4 boneIndices : BONE_INDICES,

    float4 instanceWorld0 : INSTANCE_WORLD0,
    float4 instanceWorld1 : INSTANCE_WORLD1,
    float4 instanceWorld2 : INSTANCE_WORLD2,
    float4 instanceWorld3 : INSTANCE_WORLD3)
{
    VS_OUT vout = (VS_OUT) 0;
    float4 skinned_position = SkinningPosition(position, boneWeights, boneIndices);
    float3 skinned_normal = SkinningVector(normal, boneWeights, boneIndices);
    
    //法線方向に膨らませる
    skinned_position.xyz += normalize(skinned_normal) * outlineThickness;
    
     // ★ インスタンスのワールド行列を復元して適用
    float4x4 world = float4x4(
        instanceWorld0,
        instanceWorld1,
        instanceWorld2,
        instanceWorld3
    );
    float4 worldPos = mul(skinned_position, world); // ★ ワールド適用

    vout.vertex = mul(worldPos, viewProjection); // ★ worldPos を使う
    vout.texcoord = texcoord;
    vout.normal = mul(skinned_normal, (float3x3) world); // ★ 法線もワールド変換
    vout.worldPos = worldPos.xyz; // ★ worldPos を使う
    return vout;
}