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
    XMMATRIX mWorld;            // 世界矩阵
    XMMATRIX mView;             // 观察矩阵
    XMMATRIX mProjection;       // 投影矩阵
    XMFLOAT4 vLightDir[3];      // 光线方向
    XMFLOAT4 vLightColor[3];    // 光线颜色
    XMFLOAT4 vCamera;           // 相机位置
    XMFLOAT4 vTarget;           // 看的位置
    XMFLOAT4 mColor;            // 漫反射颜色
};


// 模型
struct Model
{
    std::string mName;                  // 模型名称
    std::vector<SimpleVertex> vertices; // 顶点内存
    std::vector<WORD> indices;          // 索引内存
    std::vector<XMFLOAT4> mColors;
    std::vector<std::string> mTextureNames;
    std::vector<int> meshVertexOffset;
    std::vector<int> meshIndexOffset;
    ID3D11Buffer* mVertexBuffer;
    ID3D11Buffer* mIndexBuffer;
    ID3D11Buffer* mConstantBuffer;
};

// AABB盒
struct AABB
{
    XMFLOAT3 MaxPos;
    XMFLOAT3 MinPos;
    void SetAABB(XMMATRIX mWorld, Model* model)
    {
        // 初始化世界坐标最大最小值
        XMVECTOR worldPosVecMaxMin = XMLoadFloat3(&XMFLOAT3(model->vertices[0].Pos.x, model->vertices[0].Pos.y, model->vertices[0].Pos.z));
        worldPosVecMaxMin = XMVector3TransformCoord(worldPosVecMaxMin, mWorld);
        XMFLOAT3 worldPosFMaxMin;
        XMStoreFloat3(&worldPosFMaxMin, worldPosVecMaxMin);
        MaxPos = worldPosFMaxMin;
        MinPos = worldPosFMaxMin;
        // 寻找世界坐标最大最小值
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