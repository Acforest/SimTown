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
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
    XMFLOAT4 vLightDir[3];      // ���߷���
    XMFLOAT4 vLightColor[3];    // ������ɫ
    XMFLOAT4 vCamera;           // ���λ��
    XMFLOAT4 vTarget;           // ����λ��
    XMFLOAT4 mColor;            // ��������ɫ
};

// ģ��
struct Model
{
    std::string mName;      // ģ������
    int mNumMeshes = 0;     // mesh��
    int mNumFaces = 0;      // ����mesh������
    int mNumVertices = 0;   // ����mesh�Ķ�����
    int mNumIndices = 0;    // ����mesh��������
    SimpleVertex* vertices; // �����ڴ�
    WORD* indices;          // �����ڴ�
    std::vector<XMFLOAT4> mColors;
    std::vector<std::string> mTextureNames;
    std::vector<int> meshVertexOffset;
    std::vector<int> meshIndexOffset;
};

// AABB��
struct AABB
{
    XMFLOAT3 Center;            // �����ĵ�
    XMFLOAT3 MaxPos;
    XMFLOAT3 MinPos;
    void SetAABB(XMMATRIX mWorld, Model* model)
    {
        MaxPos = XMFLOAT3(model->vertices[0].Pos.x, model->vertices[0].Pos.y, model->vertices[0].Pos.z);
        MinPos = XMFLOAT3(model->vertices[0].Pos.x, model->vertices[0].Pos.y, model->vertices[0].Pos.z);
        for (int i = 0; i < model->mNumVertices; i++) {
            if (MaxPos.x < model->vertices[i].Pos.x) {
                MaxPos.x = model->vertices[i].Pos.x;
            }
            if (MaxPos.y < model->vertices[i].Pos.y) {
                MaxPos.y = model->vertices[i].Pos.y;
            }
            if (MaxPos.z < model->vertices[i].Pos.z) {
                MaxPos.z = model->vertices[i].Pos.z;
            }
            if (MinPos.x > model->vertices[i].Pos.x) {
                MinPos.x = model->vertices[i].Pos.x;
            }
            if (MinPos.y > model->vertices[i].Pos.y) {
                MinPos.y = model->vertices[i].Pos.y;
            }
            if (MinPos.z > model->vertices[i].Pos.z) {
                MinPos.z = model->vertices[i].Pos.z;
            }
        }
        XMVECTOR MaxPosVec = XMLoadFloat3(&MaxPos);
        MaxPosVec = XMVector3TransformCoord(MaxPosVec, mWorld);
        XMStoreFloat3(&MaxPos, MaxPosVec);
        XMVECTOR MinPosVec = XMLoadFloat3(&MinPos);
        MinPosVec = XMVector3TransformCoord(MinPosVec, mWorld);
        XMStoreFloat3(&MinPos, MinPosVec);
        Center = XMFLOAT3(0.5f * (MaxPos.x + MinPos.x), 0.5f * (MaxPos.y + MinPos.y), 0.5f * (MaxPos.z + MinPos.z));
    }
};

struct Ray
{
    XMFLOAT3 ScreenToRay(XMMATRIX mView, XMMATRIX mProj, XMVECTOR Eye, float width, float height, float screenX, float nearZ, float farZ, float screenY) {
        // ����Ļ�������ӿڱ任��NDC����ϵ
        static const XMVECTORF32 D = { { { -1.0f, 1.0f, 0.0f, 0.0f } } };
        XMVECTOR V = XMVectorSet(screenX, screenY, 0.0f, 1.0f);

        XMVECTOR Scale = XMVectorSet(width * 0.5f, -height * 0.5f, farZ - nearZ, 1.0f);
        Scale = XMVectorReciprocal(Scale);

        XMVECTOR Offset = XMVectorSet(0.0f, 0.0f, -nearZ, 0.0f);
        Offset = XMVectorMultiplyAdd(Scale, Offset, D.v);

        // ��NDC����ϵ�任����������ϵ
        XMMATRIX Transform = XMMatrixMultiply(mView, mProj);
        Transform = XMMatrixInverse(nullptr, Transform);

        XMVECTOR Target = XMVectorMultiplyAdd(V, Scale, Offset);
        Target = XMVector3TransformCoord(Target, Transform);

        // �������
        XMFLOAT3 direction;
        XMStoreFloat3(&direction, XMVector3Normalize(Target - Eye));
        return direction;
    }
};