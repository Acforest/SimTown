#pragma once
#include "BoundingBox.h"

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

// 顶点
struct SimpleVertex
{
    XMFLOAT3 Pos;       // 顶点坐标
    XMFLOAT3 Normal;    // 顶点法线
    XMFLOAT4 Color;     // 顶点颜色
    XMFLOAT2 Texture;   // 顶点纹理
};

// 物体表面材质
struct Material
{
    XMFLOAT4 Ambient;
    XMFLOAT4 Diffuse;
    XMFLOAT4 Specular;
};

// 平行光
struct DirectionalLight
{
    XMFLOAT4 Ambient;
    XMFLOAT4 Diffuse;
    XMFLOAT4 Specular;
    XMFLOAT3 Direction;
};

// 常量缓存
struct ConstantBuffer
{
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
    XMFLOAT4 vLightDir[3];      // 光线方向
    XMFLOAT4 vLightColor[3];    // 光线颜色
    XMFLOAT4 vCamera;           // 相机位置
    XMFLOAT4 vTarget;           // 看的位置
    XMFLOAT4 mColor;            // 漫反射颜色
};

// 模型
struct Model
{
    std::string mName;      // 模型名称
    int mNumMeshes = 0;     // mesh数
    int mNumFaces = 0;      // 所有mesh的面数
    int mNumVertices = 0;   // 所有mesh的顶点数
    int mNumIndices = 0;    // 所有mesh的索引数
    SimpleVertex* vertices; // 顶点内存
    WORD* indices;          // 索引内存
    std::vector<XMFLOAT4> mColors;
    std::vector<std::string> mTextureNames;
    std::vector<int> meshVertexOffset;
    std::vector<int> meshIndexOffset;
};

// AABB盒
struct AABB
{
    XMFLOAT3 Center;            // 盒中心点
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
        // 将屏幕坐标点从视口变换回NDC坐标系
        static const XMVECTORF32 D = { { { -1.0f, 1.0f, 0.0f, 0.0f } } };
        XMVECTOR V = XMVectorSet(screenX, screenY, 0.0f, 1.0f);

        XMVECTOR Scale = XMVectorSet(width * 0.5f, -height * 0.5f, farZ - nearZ, 1.0f);
        Scale = XMVectorReciprocal(Scale);

        XMVECTOR Offset = XMVectorSet(0.0f, 0.0f, -nearZ, 0.0f);
        Offset = XMVectorMultiplyAdd(Scale, Offset, D.v);

        // 从NDC坐标系变换回世界坐标系
        XMMATRIX Transform = XMMatrixMultiply(mView, mProj);
        Transform = XMMatrixInverse(nullptr, Transform);

        XMVECTOR Target = XMVectorMultiplyAdd(V, Scale, Offset);
        Target = XMVector3TransformCoord(Target, Transform);

        // 求出射线
        XMFLOAT3 direction;
        XMStoreFloat3(&direction, XMVector3Normalize(Target - Eye));
        return direction;
    }
};