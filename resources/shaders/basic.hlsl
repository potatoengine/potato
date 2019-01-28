#include "common.hlsli"

//float4x4 WorldViewProjection;

FrameData frameData : register(c0);

struct VS_Output {
    float4 position : SV_Position;
    float3 color : COLOR;
};

VS_Output vertex_main(float3 inputPosition
                      : POSITION, float3 inputColor
                      : COLOR) {
    //return mul(inputPosition, WorldViewProjection);

    float t = fmod(float(frameData.timeStamp), 2.f * PI);
    float c = cos(t);
    float s = sin(t);

    VS_Output output;
    output.position = float4(
        inputPosition.x * c - inputPosition.y * s,
        inputPosition.y * c + inputPosition.x * s,
        inputPosition.z,
        1);
    output.color = inputColor;
    return output;
}

float4 pixel_main(VS_Output input) : SV_Target {
    return float4(input.color, 1);
}
