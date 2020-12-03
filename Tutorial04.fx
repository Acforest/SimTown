//--------------------------------------------------------------------------------------
// File: Tutorial04.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
    float4 vLightDir;
    float4 vLightColor;
    float4 vCamera;
    float4 vLookAt;
}

//--------------------------------------------------------------------------------------

struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Color : COLOR;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    output.Norm = mul(input.Norm, (float3x4) World);
    output.Color = input.Color;
    output.Tex = input.Tex;
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS_Ambient_Shading(PS_INPUT input) : SV_Target
{
    return input.Color;
}

float4 PS_Lambertian_Shading(PS_INPUT input) : SV_Target
{
    float4 finalColor = saturate(input.Color * vLightColor * dot(normalize((float3) vLightDir), input.Norm) + float4(0.1, 0.1, 0.1, 1.0));
	finalColor.a = 1.0f;
    return finalColor;
}

float4 PS_Blinn_Phong_Shading(PS_INPUT input) : SV_Target
{
    float4 finalColor;
    float3 bisector = normalize((float3) (normalize(vCamera - input.Pos) + vLightDir)); // 相机视方向与光线方向的二分线
    int x = 5;  // 高光指数
    float4 sc = { 1.0f, 1.0f, 1.0f, 1.0f }; // 高光颜色
    finalColor = input.Color * vLightColor * max(dot(normalize((float3) vLightDir), input.Norm), 0) + sc * vLightColor * pow(max(dot(bisector, input.Norm), 0), x);
    finalColor.a = 1.0f;
    return finalColor;
}

float4 PS_Toon_Shading(PS_INPUT input) : SV_Target
{
    float cos_theta = max(dot(normalize((float3) vLightDir), input.Norm), 0);
	float4 finalColor;
	if (cos_theta < 0.25)
	{
        finalColor = float4(0.0f, 0.65f, 0.0f, 1.0f);
    }
	else if (cos_theta < 0.5)
	{
        finalColor = float4(0.0f, 0.7f, 0.0f, 1.0f);
    }
	else if (cos_theta < 0.75)
	{
        finalColor = float4(0.0f, 0.85f, 0.0f, 1.0f);
    }
	else
	{
        finalColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
    }
    finalColor.a = 1.0f;
	return finalColor;
}

float4 PS_Texture_Mapping(PS_INPUT input) : SV_Target
{
    float4 tex_color = txDiffuse.Sample(samLinear, input.Tex);
    float4 finalColor = saturate(tex_color * dot(normalize((float3) vLightDir), input.Norm));
    finalColor.a = 1.0f;
    return finalColor;
}