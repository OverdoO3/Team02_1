#include "Lambert.hlsli"
#include "Skinning.hlsli"

struct VS_IN
{
    float4 position : POSITION;
    float4 boneWeights : BONE_WEIGHTS;
    uint4 boneIndices : BONE_INDICES;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;

    // ★ ここ追加（Instance用）
    float4 instanceWorld0 : INSTANCE_WORLD0;
    float4 instanceWorld1 : INSTANCE_WORLD1;
    float4 instanceWorld2 : INSTANCE_WORLD2;
    float4 instanceWorld3 : INSTANCE_WORLD3;
};

VS_OUT main(VS_IN input)
{
    VS_OUT vout = (VS_OUT) 0;

    // スキニング
    float4 pos = SkinningPosition(input.position, input.boneWeights, input.boneIndices);
    float3 nrm = SkinningVector(input.normal, input.boneWeights, input.boneIndices);

    // ワールド行列復元
    float4x4 world =
        float4x4(
            input.instanceWorld0,
            input.instanceWorld1,
            input.instanceWorld2,
            input.instanceWorld3
        );

    // ワールド適用
    float4 worldPos = mul(pos, world);

    // ViewProjection
    //vout.vertex = mul(worldPos, viewProjection);
    vout.vertex = mul(worldPos, viewProjection);

    vout.texcoord = input.texcoord;
    vout.normal = nrm;

    return vout;
}