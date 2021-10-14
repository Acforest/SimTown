#pragma once
#include "Main.h"
//--------------------------------------------------------------------------------------
// ������պ�����
//--------------------------------------------------------------------------------------
void CreateSphere(float radius, UINT levels, UINT slices)
{
    // ���������Ķ������ݺ��������ݣ���ʱ����ƣ�
    UINT vertexCount = 2 + (levels - 1) * (slices + 1);
    UINT indexCount = 6 * (levels - 1) * slices;
    skyVertices.resize(vertexCount);
    skyIndices.resize(indexCount);

    SimpleVertex vertexData;
    UINT vIndex = 0, iIndex = 0;

    float phi = 0.0f, theta = 0.0f;
    float per_phi = XM_PI / levels;
    float per_theta = XM_2PI / slices;
    float x, y, z;

    // ���붥�˵�
    vertexData = { XMFLOAT3(0.0f, radius, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) };
    skyVertices[vIndex++] = vertexData;

    for (UINT i = 1; i < levels; ++i)
    {
        phi = per_phi * i;
        // ��Ҫslices + 1����������Ϊ �����յ���Ϊͬһ�㣬����������ֵ��һ��
        for (UINT j = 0; j <= slices; ++j)
        {
            theta = per_theta * j;
            x = radius * sinf(phi) * cosf(theta);
            y = radius * cosf(phi);
            z = radius * sinf(phi) * sinf(theta);
            // ������ֲ����ꡢ��������Tangent��������������
            XMFLOAT3 pos = XMFLOAT3(x, y, z), normal;
            XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&pos)));
            //vertexData = { pos, normal, XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f), XMFLOAT2(theta / XM_2PI, phi / XM_PI) };
            //Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);
        }
    }

    // ����׶˵�
    vertexData = { XMFLOAT3(0.0f, -radius, 0.0f) };
    skyVertices[vIndex++] = vertexData;

    // ��������
    if (levels > 1)
    {
        for (UINT j = 1; j <= slices; ++j)
        {
            skyIndices[iIndex++] = 0;
            skyIndices[iIndex++] = j;
            skyIndices[iIndex++] = j % (slices + 1) + 1;
        }
    }

    for (UINT i = 1; i < levels - 1; ++i)
    {
        for (UINT j = 1; j <= slices; ++j)
        {
            skyIndices[iIndex++] = (i - 1) * (slices + 1) + j;
            skyIndices[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;
            skyIndices[iIndex++] = (i - 1) * (slices + 1) + j % (slices + 1) + 1;

            skyIndices[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;
            skyIndices[iIndex++] = (i - 1) * (slices + 1) + j;
            skyIndices[iIndex++] = i * (slices + 1) + j;
        }
    }

    if (levels > 1)
    {
        for (UINT j = 1; j <= slices; ++j)
        {
            skyIndices[iIndex++] = (levels - 2) * (slices + 1) + j;
            skyIndices[iIndex++] = (levels - 1) * (slices + 1) + 1;
            skyIndices[iIndex++] = (levels - 2) * (slices + 1) + j % (slices + 1) + 1;
        }
    }
}

void RenderSky()
{
    g_pImmediateContext->VSSetShader(g_pSkyVertexShader, nullptr, 0);			// ���ö�����ɫ��
    g_pImmediateContext->PSSetShader(g_pPixelShaders[4], nullptr, 0);		// ����������ɫ��
    g_pImmediateContext->GSSetShader(nullptr, nullptr, 0);
    g_pImmediateContext->OMSetDepthStencilState(*g_pDSS, 0);		// �������/ģ��״̬
    g_pImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

    UINT strides = sizeof(SimpleVertex);
    UINT offsets = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pSkyVertexBuffer, &strides, &offsets);	// ���ö��㻺����
    g_pImmediateContext->IASetIndexBuffer(g_pSkyIndexBuffer, DXGI_FORMAT_R32_UINT, 0);		// ��������������
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pSkyConstantBuffer); // ���������󶨵���Ⱦ������
    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureCubeSRVs[0]); // ����SRV
    g_pImmediateContext->DrawIndexed(skyIndices.size(), 0, 0); // ����
}