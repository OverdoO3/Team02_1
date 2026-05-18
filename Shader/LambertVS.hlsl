#include "Lambert.hlsli"
#include "Skinning.hlsli"

struct VS_IN
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
    float4 boneWeights : BONE_WEIGHTS;
    uint4 boneIndices : BONE_INDICES;

    // インスタンシング用ワールド行列
    float4 instanceWorld0 : INSTANCE_WORLD0;
    float4 instanceWorld1 : INSTANCE_WORLD1;
    float4 instanceWorld2 : INSTANCE_WORLD2;
    float4 instanceWorld3 : INSTANCE_WORLD3;
};

VS_OUT main(VS_IN input)
{
    VS_OUT vout = (VS_OUT) 0;

    // スキニング
    float4 skinnedPos = SkinningPosition(input.position, input.boneWeights, input.boneIndices);
    float3 skinnedNrm = SkinningVector(input.normal, input.boneWeights, input.boneIndices);

    // インスタンスごとのワールド行列を復元
    float4x4 world = float4x4(
        input.instanceWorld0,
        input.instanceWorld1,
        input.instanceWorld2,
        input.instanceWorld3
    );

    // ワールド空間へ変換
    float4 worldPos = mul(skinnedPos, world);

    // クリップ空間
    vout.vertex = mul(worldPos, viewProjection);

    // 影用クリップ空間（ライト視点）
    vout.shadow_position = mul(worldPos, lightViewProjection);

    // テクスチャ座標
    vout.texcoord = input.texcoord;

    // 法線（ワールド空間）
    vout.normal = mul(skinnedNrm, (float3x3) world);

    // PS でのライティング計算用ワールド座標
    vout.position = worldPos.xyz;
    vout.worldPos = worldPos.xyz;

    return vout;
}
