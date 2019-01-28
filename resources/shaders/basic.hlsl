#include "common.hlsli"

//float4x4 WorldViewProjection;

struct VS_Output {
    float4 position : SV_Position;
    float3 color : COLOR;
};

VS_Output vertex_main(float3 inputPosition
                      : POSITION, float3 inputColor
                      : COLOR) {
    VS_Output output;
    output.position = mul(float4(inputPosition, 1), worldViewProjection);
    output.color = inputColor;
    return output;
}

float4 pixel_main(VS_Output input) : SV_Target {
    return float4(input.color, 1);
}
