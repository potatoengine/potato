// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

cbuffer frameData : register(b0) {
    uint frameNumber;
    float lastFrameTimeDelta;
    double timeStamp;
};

cbuffer cameraData : register(b1) {
    float4x4 worldViewProjection row;
    float4x4 worldView;
    float4x4 viewProjection;
};

static const float PI = 3.14159265f;
