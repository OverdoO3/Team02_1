#include "Lambert.hlsli"
#include "Skinning.hlsli"

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
    
    vout.vertex = mul(skinned_position, viewProjection);
    vout.shadow_position = mul(skinned_position, lightViewProjection);

    vout.texcoord = texcoord;
    vout.normal = SkinningVector(normal, boneWeights, boneIndices);
    vout.position = skinned_position.xyz; // PS‚Å‚ÌŒvŽZ—p
    vout.worldPos = skinned_position.xyz;
    
    return vout;
}