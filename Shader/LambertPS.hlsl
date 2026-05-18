#include "Lambert.hlsli"

cbuffer MaterialColor : register(b1)
{
    float4 materialColor;
    
    //float shadow_color;
    //float shadow_bias;
    //float2 padding;

};

cbuffer CbShadowParams : register(b4)
{
    float shadow_color;
    float shadow_bias;
    float2 padding2;
}

Texture2D DiffuseMap : register(t0);
SamplerState LinearSampler : register(s0);

// TODO
Texture2D shadowmap : register(t8);
SamplerComparisonState shadowSampler : register(s8);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = DiffuseMap.Sample(LinearSampler, pin.texcoord);
    float3 E = normalize(pin.worldPos.xyz - cameraPosition.xyz);
    float3 N = normalize(pin.normal);
    float3 L = normalize(-lightDirection.xyz);


    float3 shadow_coords = pin.shadow_position.xyz / pin.shadow_position.w;
    float2 shadow_uv = shadow_coords.xy * float2(0.5f, -0.5f) + 0.5f;

    
    //float shadow_bias = 0.0001;
    //float shadow_color = 0.5;
    float compareDepth = shadow_coords.z - shadow_bias;
    
    float shadowIntensity = shadowmap.SampleCmpLevelZero(
        shadowSampler,
        shadow_uv,
        compareDepth,
        0
    );
    
    float shadowFactor = lerp(shadow_color, 1.0f, shadowIntensity);

    float3 directional_diffuse = CalcLambert(N, L, lightColor.rgb * lightIntensity);
    float3 directional_specular = ClacHalfLambert(N, L, lightColor.rgb * lightIntensity, float3(1, 1, 1), contrastPower);
    //ì_åıåπÇÃèàóù
    float3 point_diffuse = 0;
    float3 point_specular = 0;
    for (int i = 0; i < 8; ++i)
    {
        float3 LP = point_light[i].position.xyz - pin.worldPos.xyz;
        float len = length(LP);
        if (len >= point_light[i].range)
            continue;
        float attenuateLength = saturate(1.0f - len / point_light[i].range);
        float attenuation = attenuateLength * attenuateLength * attenuateLength;
        LP /= len;
    
        float NdotL = dot(N, LP) * 0.5f + 0.5f;
        float power = NdotL * NdotL;
        point_diffuse += point_light[i].color.rgb * pointLightIntensity * power * attenuation;
    
        point_specular += CalcPhongSpecular(N, LP, E, point_light[i].color.rgb) * attenuation;
    }
    float4 Color = float4(ambientColor.rgb, color.a);
    Color.rgb += (directional_diffuse + point_diffuse);
    Color.rgb += directional_specular + point_specular;
    Color.rgb *= color.rgb;

    Color.rgb *= shadowFactor;
    
    return Color;
}