//--------------------------------------------------------------------------------------
// File: Tutorial04.fx
TextureCube g_TexCube : register(t0);
Texture2D txDiffuse : register(t1);
Texture2D g_ShadowMap : register(t2);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
    float4 vLightDir[3];
    float4 vLightColor[3];
    float4 vCamera;
    float4 vTarget;
    float4 mColor;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
    float4 PosH : SV_POSITION;
    float4 PosW : POSITION0;
    float4 PosL : POSITION1;
    float4 ShadowPosH : POSITION2;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    output.PosH = mul(input.Pos, World);
    output.PosH = mul(output.PosH, View);
    output.PosH = mul(output.PosH, Projection);
    output.PosW = mul(input.Pos, World);
    output.PosL = input.Pos;
    output.Norm = mul(input.Norm, (float3x4) World);
    output.Tex = input.Tex;
    //output.ShadowPosH = mul(output.PosW, );
    return output;
}

PS_INPUT Sky_VS(VS_INPUT input)
{
    PS_INPUT output;
    output.PosL = input.Pos;
    output.PosH = mul(input.Pos, World);
    output.PosH = mul(output.PosH, View);
    output.PosH = mul(output.PosH, Projection);
    // ����z = wʹ��z/w = 1(��պб�����Զƽ��)
    output.PosH.z = output.PosH.w;
    output.Tex = input.Tex;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS_Ambient_Shading(PS_INPUT input) : SV_Target
{
    //input.ShadowPosH.xyz /= input.ShadowPosH.z;
    //float depth = input.ShadowPosH.z;
    //float4 color = float4(depthValue, depthValue, depthValue, 1.0f);
    return mColor;
}

float4 PS_Lambertian_Shading(PS_INPUT input) : SV_Target
{
    float4 finalColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    for (int i = 0; i < 3; i++)
    {
        finalColor += saturate(dot(normalize((float3) vLightDir[i]), input.Norm) * vLightColor[i] * mColor);
    }
    finalColor = saturate(finalColor);
	finalColor.a = 1.0f;
    return finalColor;
}

float4 PS_Blinn_Phong_Shading(PS_INPUT input) : SV_Target
{
    float4 finalColor = 0.0f;
    float4 v = normalize(vTarget + vCamera);
    for (int i = 0; i < 3; i++)
    {
        float3 bisector = normalize((float3) (v + vLightDir[i])); // ����ӷ�������߷���Ķ�����
        int x = 5; // �߹�ָ��
        float4 sc = { 1.0f, 1.0f, 1.0f, 1.0f }; // �߹���ɫ
        finalColor += mColor * vLightColor[i] * max(dot(normalize((float3) vLightDir[i]), input.Norm), 0) + sc * vLightColor[i] * pow(max(dot(bisector, input.Norm), 0), x);
    }
    finalColor = saturate(finalColor);
    finalColor.a = 1.0f;
    return finalColor;
}

float4 PS_Toon_Shading(PS_INPUT input) : SV_Target
{
    float cos_theta = max(dot(normalize((float3) vLightDir[0]), input.Norm), 0);
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
    float4 finalColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    float4 tex_color = txDiffuse.Sample(samLinear, input.Tex);
    float2 shadowTex = float2(0.5f * input.PosH.x / input.PosH.w + 0.5f, -0.5f * input.PosH.y / input.PosH.w + 0.5f);
    
    if (saturate(shadowTex.x) == shadowTex.x && saturate(shadowTex.y) == shadowTex.y)
    {
        
    }
    return tex_color * mColor;
}

float4 PS_Sky_Texture(PS_INPUT input) : SV_Target
{
    return g_TexCube.Sample(samLinear, input.PosL);
}
