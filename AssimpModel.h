#pragma once

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
    XMFLOAT4 mColor;
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
    std::vector<int> meshVertexOffset;
    std::vector<int> meshIndexOffset;
};