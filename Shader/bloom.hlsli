// BLOOM
cbuffer BLOOM_CONSTAT_BUFFER : register(b9)
{
    float bloom_extraction_threshold;
    float bloom_intensity;
    float2 padding2;
}