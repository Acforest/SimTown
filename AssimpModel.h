#pragma once
#include "BoundingBox.h"

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

// ����
struct SimpleVertex
{
    XMFLOAT3 Pos;       // ��������
    XMFLOAT3 Normal;    // ���㷨��
    XMFLOAT4 Color;     // ������ɫ
    XMFLOAT2 Texture;   // ��������
};

// ����������
struct Material
{
    XMFLOAT4 Ambient;
    XMFLOAT4 Diffuse;
    XMFLOAT4 Specular;
};

// ƽ�й�
struct DirectionalLight
{
    XMFLOAT4 Ambient;
    XMFLOAT4 Diffuse;
    XMFLOAT4 Specular;
    XMFLOAT3 Direction;
};

// ��������
struct ConstantBuffer
{
    XMMATRIX mWorld;            // �������
    XMMATRIX mView;             // �۲����
    XMMATRIX mProjection;       // ͶӰ����
    XMFLOAT4 vLightDir[3];      // ���߷���
    XMFLOAT4 vLightColor[3];    // ������ɫ
    XMFLOAT4 vCamera;           // ���λ��
    XMFLOAT4 vTarget;           // ����λ��
    XMFLOAT4 mColor;            // ��������ɫ
};


// ģ��
struct Model
{
    std::string mName;                  // ģ������
    std::vector<SimpleVertex> vertices; // �����ڴ�
    std::vector<WORD> indices;          // �����ڴ�
    std::vector<XMFLOAT4> mColors;
    std::vector<std::string> mTextureNames;
    std::vector<int> meshVertexOffset;
    std::vector<int> meshIndexOffset;
    ID3D11Buffer* mVertexBuffer;
    ID3D11Buffer* mIndexBuffer;
    ID3D11Buffer* mConstantBuffer;
};

// AABB��
struct AABB
{
    XMFLOAT3 MaxPos;
    XMFLOAT3 MinPos;
    void SetAABB(XMMATRIX mWorld, Model* model)
    {
        // ��ʼ���������������Сֵ
        XMVECTOR worldPosVecMaxMin = XMLoadFloat3(&XMFLOAT3(model->vertices[0].Pos.x, model->vertices[0].Pos.y, model->vertices[0].Pos.z));
        worldPosVecMaxMin = XMVector3TransformCoord(worldPosVecMaxMin, mWorld);
        XMFLOAT3 worldPosFMaxMin;
        XMStoreFloat3(&worldPosFMaxMin, worldPosVecMaxMin);
        MaxPos = worldPosFMaxMin;
        MinPos = worldPosFMaxMin;
        // Ѱ���������������Сֵ
        for (int i = 0; i < model->vertices.size(); i++) {
            XMVECTOR worldPosVec = XMLoadFloat3(&XMFLOAT3(model->vertices[i].Pos.x, model->vertices[i].Pos.y, model->vertices[i].Pos.z));
            worldPosVec = XMVector3TransformCoord(worldPosVec, mWorld);
            XMFLOAT3 worldPosF;
            XMStoreFloat3(&worldPosF, worldPosVec);

            if (MaxPos.x < worldPosF.x) {
                MaxPos.x = worldPosF.x;
            }
            if (MaxPos.y < worldPosF.y) {
                MaxPos.y = worldPosF.y;
            }
            if (MaxPos.z < worldPosF.z) {
                MaxPos.z = worldPosF.z;
            }
            if (MinPos.x > worldPosF.x) {
                MinPos.x = worldPosF.x;
            }
            if (MinPos.y > worldPosF.y) {
                MinPos.y = worldPosF.y;
            }
            if (MinPos.z > worldPosF.z) {
                MinPos.z = worldPosF.z;
            }
        }
    }
};