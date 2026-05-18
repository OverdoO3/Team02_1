#include "lights.hlsli"

//------------------------
//  ランバート拡散反射計算関数
//------------------------
// N:法線(正規化済み)
// C:入射ベクトル(正規化済み)
// K:反射率
float3 CalcLambert(float3 N, float3 L, float3 C)
{
    float power = saturate(dot(N, -L));
    return C * power;
}

//------------------------
//  フォンの鏡面反射計算関数
//------------------------
// N:法線(正規化済み)
// L:入射ベクトル(正規済み)
// E:視線ベクトル(正規化済み)
// C:入射工(色・強さ)
// K:反射率
float3 CalcPhongSpecular(float3 N, float3 L, float3 E, float3 C)
{
    float3 R = reflect(L, N);
    float power = max(dot(-E, R), 0);
    power = pow(power, 128);
    return C * power;
}

//-----------------------------
// ハーフランドバート拡散反射計算関数
//-----------------------------
// N:法線(正規化済み)
// L:入射ベクトル(正規化済み)
// C:入射光(色・強さ)
// K:反射率
float3 ClacHalfLambert(float3 N, float3 L, float3 C, float3 K, float contrastPower)
{
    float D = saturate(dot(N, -L) * 0.5f + 0.5f);
    
    float D_adj = pow(D, contrastPower);
    return C * D_adj * K;
}

//----------------------------
// ランプシェーディング
//----------------------------
// tex:ランプシェーディング用テクスチャ
// samp:ランプシェーディング用サンプラステート
// N:法線(正規化済み)
// L:入射ベクトル(正規化済み)
// C:入射光(色・強さ)
// K:反射率
float3 CalcRampShading(Texture2D tex, SamplerState samp, float3 N, float3 L, float3 C, float3 K)
{
    float D = saturate(dot(N, -L) * 0.5f + 0.5f);
    float Ramp = tex.Sample(samp, float2(D, 0.5f).r);
    return C * Ramp * K.rgb;
}

//----------------------------
// 半球ライティング
//----------------------------
// normal:法線(正規化済み)
// up:上方向(片方)
// sky_color:空(上)色
// ground_color:地面(下)色
// hemisphere_weight
float3 CalcHemiSphereLight(float3 normal, float3 up, float3 sky_color, float3 ground_color, float4 hemisphere_weight)
{
    float factor = dot(normal, up) * 0.5f + 0.5f;
    return lerp(ground_color, sky_color, factor) * hemisphere_weight.x;
}


cbuffer CbScene : register(b3)
{
    row_major float4x4 viewProjection;
    float4 cameraPosition;
};


cbuffer CbLight : register(b2)
{
    row_major float4x4 lightViewProjection;
    float4 lightDirection;
    float4 lightColor;
    float4 ambientColor;
    point_lights point_light[8];
}

cbuffer CBLightParams : register(b5)
{
    float lightIntensity; //明るさ調整
    float contrastPower; //標準1.0 
    float pointLightIntensity;
    float padding;
}