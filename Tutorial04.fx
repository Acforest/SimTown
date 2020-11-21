//--------------------------------------------------------------------------------------
// File: Tutorial04.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
    float4 vLightDir;
    float4 vLightColor;
    float4 vLightPos;
    float4 vOutputColor;
    float4 vCamera;
}

//--------------------------------------------------------------------------------------

struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Color : COLOR;
    float3 Norm : NORMAL;
    //float4 LightPos : POSITION;
    //float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float3 Norm : NORMAL;
    //float4 LightPos : POSITION;
    //float2 Tex : TEXCOORD0;
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
    output.Color = input.Color;
    
    output.Norm = mul(input.Norm, (float3x4) World);
    //output.Tex = input.Tex;
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS_Ambient_Shading(PS_INPUT input) : SV_Target
{
    return float4(0.0f, 1.0f, 0.0f, 1.0f);
}

float4 PS_Lambertian_Shading(PS_INPUT input) : SV_Target
{
    float4 finalColor = saturate(input.Color * vLightColor * dot(normalize((float3) vLightDir), input.Norm));
	finalColor.a = 1.0f;
    return finalColor;
}

float4 PS_Blinn_Phong_Shading(PS_INPUT input) : SV_Target
{
    float4 finalColor;
    float4 bisector = normalize(vCamera + vLightDir);
    float4 sc = { 1.0f, 1.0f, 1.0f, 1.0f };
    finalColor = saturate(input.Color * vLightColor * max(dot(normalize((float3) vLightDir), input.Norm), 0) + vLightColor * sc * pow(max(dot((float3) bisector, input.Norm), 0), 5));
    finalColor.a = 1.0f;
    return finalColor;
}

float4 PS_Toon_Shading(PS_INPUT input) : SV_Target
{
    float cos_theta = max(dot(normalize((float3) vLightDir), input.Norm), 0);
	float4 finalColor;
	if (cos_theta < 0.25)
	{
        finalColor = saturate(input.Color * float4(1.0f, 1.0f, 1.0f, 1.0f));
    }
	else if (cos_theta < 0.5)
	{
        finalColor = saturate(input.Color * float4(1.0f, 1.0f, 1.0f, 1.0f));
    }
	else if (cos_theta < 0.75)
	{
        finalColor = saturate(input.Color * float4(1.0f, 1.0f, 1.0f, 1.0f));
    }
	else
	{
		finalColor = input.Color;
	}
    finalColor.a = 1.0f;
	return finalColor;
}

/*float4 PS(PS_INPUT input) : SV_Target
{
    return txDiffuse.Sample(samLinear, input.Tex) * vMeshColor;
}*/