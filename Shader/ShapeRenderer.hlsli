struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer CbMesh : register(b5)
{
    row_major float4x4 worldViewProjection;
    float4 color;
    float4 ka;
    float4 kd;
    float4 ks;
    
    float shadow_color;
    float shadow_bias;
    float2 padding;
};