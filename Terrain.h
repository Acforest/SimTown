#pragma once
#include "Main.h"

class Terrain
{
private:
    std::vector<float> m_heightInfo;        // �߶�ͼ�߶���Ϣ
    int m_cellsPerRow;                      // ÿ�е�Ԫ����
    int m_cellsPerCol;                      // ÿ�е�Ԫ����
    int m_verticesPerRow;                   // ÿ�ж�����
    int m_verticesPerCol;                   // ÿ�ж�����
    int m_numsVertices;                     // ��������
    float m_width;                          // ���ο��
    float m_height;                         // ���θ߶�
    float m_heightScale;                    // �߶�����ϵ��
    ID3D11Buffer* m_pVertexBuffer;
    ID3D11Buffer* m_pIndexBuffer;
    ID3D11Buffer* m_pConstantBuffer;
    ID3D11InputLayout* m_pInputLayout;
    ID3D11ShaderResourceView* m_pSRVTerrain;
    ID3D11SamplerState* m_pSamplerLinear;
public:
    std::vector<SimpleVertex> m_vertices;   // ���㼯��
    std::vector<WORD> m_indices;            // ��������
    int vertex_offset;
    int index_offset;
    Terrain();
    // ���ظ߶�ͼ
    void LoadHeightMap(std::string filePath);
    // ���㷨��
    void ComputeNormal(SimpleVertex& v1, SimpleVertex& v2, SimpleVertex& v3, XMFLOAT3& normal);
    // ��ʼ������
    void InitTerrain(float width, float height, UINT m, UINT n, float scale);
    // ����������
    void BuildBuffer();
    // ��������
    void BuildSRVs(const wchar_t* texturePath);
    // �������벼��
    void BuildInputLayouts();
    // ��Ⱦ����
    void Render(ConstantBuffer& cb, long vertex_offset, long index_offset);
};