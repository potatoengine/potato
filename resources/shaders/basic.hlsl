#include "common.hlsli"

//float4x4 WorldViewProjection;

float4 vertex_main(float4 inputPosition
                   : POSITION) : SV_Position {
    //return mul(inputPosition, WorldViewProjection);
    return inputPosition;
}

float4 pixel_main(float4 inputPosition
                  : SV_Position) : SV_Target {
    return float4(inputPosition.xyz, 1);
}
