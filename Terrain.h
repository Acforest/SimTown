#pragma once
#include "Main.h"

class Terrain
{
private:
    std::vector<float> m_heightInfo;        // 高度图高度信息
    int m_cellsPerRow;                      // 每行单元格数
    int m_cellsPerCol;                      // 每列单元格数
    int m_verticesPerRow;                   // 每行顶点数
    int m_verticesPerCol;                   // 每列顶点数
    int m_numsVertices;                     // 顶点总数
    float m_width;                          // 地形宽度
    float m_height;                         // 地形高度
    float m_heightScale;                    // 高度缩放系数
    ID3D11Buffer* m_pVertexBuffer;
    ID3D11Buffer* m_pIndexBuffer;
    ID3D11Buffer* m_pConstantBuffer;
    ID3D11InputLayout* m_pInputLayout;
    ID3D11ShaderResourceView* m_pSRVTerrain;
    ID3D11SamplerState* m_pSamplerLinear;
public:
    std::vector<SimpleVertex> m_vertices;   // 顶点集合
    std::vector<WORD> m_indices;            // 索引集合
    int vertex_offset;
    int index_offset;
    Terrain();
    // 加载高度图
    void LoadHeightMap(std::string filePath);
    // 计算法线
    void ComputeNormal(SimpleVertex& v1, SimpleVertex& v2, SimpleVertex& v3, XMFLOAT3& normal);
    // 初始化地形
    void InitTerrain(float width, float height, UINT m, UINT n, float scale);
    // 建立缓冲区
    void BuildBuffer();
    // 建立纹理
    void BuildSRVs(const wchar_t* texturePath);
    // 建立输入布局
    void BuildInputLayouts();
    // 渲染地形
    void Render(ConstantBuffer& cb, long vertex_offset, long index_offset);
};