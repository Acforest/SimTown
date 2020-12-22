#pragma once

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
    XMFLOAT4 mColor;
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
    std::vector<int> meshVertexOffset;
    std::vector<int> meshIndexOffset;
};