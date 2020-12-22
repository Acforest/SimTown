#pragma once
#include "Main.h"
//--------------------------------------------------------------------------------------
// 创建天空盒球体
//--------------------------------------------------------------------------------------
void CreateSphere(float radius, UINT levels, UINT slices)
{
    // 创建天空球的顶点数据和索引数据（逆时针绘制）
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

    // 放入顶端点
    vertexData = { XMFLOAT3(0.0f, radius, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) };
    skyVertices[vIndex++] = vertexData;

    for (UINT i = 1; i < levels; ++i)
    {
        phi = per_phi * i;
        // 需要slices + 1个顶点是因为 起点和终点需为同一点，但纹理坐标值不一致
        for (UINT j = 0; j <= slices; ++j)
        {
            theta = per_theta * j;
            x = radius * sinf(phi) * cosf(theta);
            y = radius * cosf(phi);
            z = radius * sinf(phi) * sinf(theta);
            // 计算出局部坐标、法向量、Tangent向量和纹理坐标
            XMFLOAT3 pos = XMFLOAT3(x, y, z), normal;
            XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&pos)));
            //vertexData = { pos, normal, XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f), XMFLOAT2(theta / XM_2PI, phi / XM_PI) };
            //Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);
        }
    }

    // 放入底端点
    vertexData = { XMFLOAT3(0.0f, -radius, 0.0f) };
    skyVertices[vIndex++] = vertexData;

    // 放入索引
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
    g_pImmediateContext->VSSetShader(g_pSkyVertexShader, nullptr, 0);			// 设置顶点着色器
    g_pImmediateContext->PSSetShader(g_pPixelShaders[4], nullptr, 0);		// 设置像素着色器
    g_pImmediateContext->GSSetShader(nullptr, nullptr, 0);
    g_pImmediateContext->OMSetDepthStencilState(*g_pDSS, 0);		// 设置深度/模板状态
    g_pImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

    UINT strides = sizeof(SimpleVertex);
    UINT offsets = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pSkyVertexBuffer, &strides, &offsets);	// 设置顶点缓冲区
    g_pImmediateContext->IASetIndexBuffer(g_pSkyIndexBuffer, DXGI_FORMAT_R32_UINT, 0);		// 设置索引缓冲区
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pSkyConstantBuffer); // 将缓冲区绑定到渲染管线上
    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureCubeSRVs[0]); // 设置SRV
    g_pImmediateContext->DrawIndexed(skyIndices.size(), 0, 0); // 绘制
}