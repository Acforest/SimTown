//--------------------------------------------------------------------------------------
// File: Tutorial04.cpp
//
// This application displays a 3D cube using Direct3D 11
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <cimport.h>
#include <scene.h>
#include <postprocess.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "Resource.h"
#include "Keyboard.h"
#include "Mouse.h"


int mNumVertices = 0; // �ܶ�����
int mNumIndices = 0;  // ��������

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;       // ��������
    XMFLOAT3 Normal;    // ���㷨��
    XMFLOAT4 Color;     // ������ɫ
    XMFLOAT2 Texture;   // ��������
};


struct ConstantBuffer
{
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
    XMFLOAT4 vLightDir[3];      // ���߷���
    XMFLOAT4 vLightColor[3];    // ������ɫ
    XMFLOAT4 vCamera;           // ���λ��
};


struct Model
{
    std::string mName;      // ģ������
    int mNumMeshes = 0;     // mesh��
    int mNumFaces = 0;      // ����mesh������
    int mNumVertices = 0;   // ����mesh�Ķ�����
    int mNumIndices = 0;    // ����mesh��������
    SimpleVertex* vertices; // �����ڴ�
    WORD* indices;          // �����ڴ�
};


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pImmediateContext = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11Texture2D* g_pDepthStencil = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;
ID3D11VertexShader* g_pVertexShader = NULL;
ID3D11PixelShader* g_pPixelShader = NULL;
ID3D11InputLayout* g_pVertexLayout = NULL;
ID3D11Buffer* g_pVertexBuffer = NULL;
ID3D11Buffer* g_pIndexBuffer = NULL;
ID3D11Buffer* g_pConstantBuffer = NULL;
ID3D11ShaderResourceView* g_pTextureRV = NULL;
ID3D11SamplerState* g_pSamplerLinear = NULL;
ID3D11DepthStencilState** g_pDSS;
XMMATRIX                g_World;
XMMATRIX                g_View;
XMMATRIX                g_Projection;
ConstantBuffer          cb;
ID3D11ShaderResourceView* m_pTexture = NULL;

std::vector<Model*> models; // ��ż��ص�����ģ��
std::vector<ID3D11PixelShader*> g_pPixelShaders(5, NULL);   // ��ż��ص����� Pixel Shader
std::vector<XMMATRIX> m_Worlds; // ������м��ص�ģ�͵��������
std::unique_ptr<DirectX::Keyboard> m_pKeyboard = std::make_unique<DirectX::Keyboard>(); // ���̵���
std::unique_ptr<DirectX::Mouse> m_pMouse = std::make_unique<DirectX::Mouse>(); // ��굥��
DirectX::Mouse::ButtonStateTracker m_MouseTracker; // ���״̬׷��
std::vector<SimpleVertex> skyVertices; // ��պж���
std::vector<WORD> skyIndices;  // ��պ�����
XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
XMVECTOR At = XMVectorSet(0.0f, 1.0f, 5.0f, 0.0f);
XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void Render();

// ��һ�˳����
class FirstPersonCamera
{
private:
    XMFLOAT4 mPosition;     // λ��
    XMFLOAT4 mTarget;       // ����λ��
    XMFLOAT4 mUpAxis;       // �Ϸ��򣨾ֲ�����ϵ��+Y�ᣩ
    XMFLOAT4 mRightAxis;    // �ҷ��򣨾ֲ�����ϵ��+X�ᣩ
public:
    FirstPersonCamera(XMVECTOR& mPosition, XMVECTOR& mTarget, XMVECTOR& mUpAxis, XMVECTOR& mRightAxis)
    {
        XMStoreFloat4(&this->mPosition, mPosition);
        XMStoreFloat4(&this->mTarget, mTarget);
        XMStoreFloat4(&this->mUpAxis, mUpAxis);
        XMStoreFloat4(&this->mRightAxis, mRightAxis);
    }
    XMFLOAT4 GetPosition()
    {
        return mPosition;
    }
    XMFLOAT4 GetTarget()
    {
        return mTarget;
    }
    XMFLOAT4 GetUpAxis()
    {
        return mUpAxis;
    }
    XMFLOAT4 GetRightAxis()
    {
        return mRightAxis;
    }
    // �����ƶ�
    void MoveLeftRight(float d)
    {
        float normalize_Length = mRightAxis.x * mRightAxis.x + mRightAxis.z * mRightAxis.z;
        float normalize_X = mRightAxis.x / normalize_Length;
        float normalize_Z = mRightAxis.z / normalize_Length;
        mPosition.x += normalize_X * d;
        mPosition.z += normalize_Z * d;
    }
    // ǰ���ƶ�
    void MoveForwardBack(float d)
    {
        float normalize_Length = mTarget.x * mTarget.x + mTarget.z * mTarget.z;
        float normalize_X = mTarget.x / normalize_Length;
        float normalize_Z = mTarget.z / normalize_Length;
        mPosition.x += normalize_X * d;
        mPosition.z += normalize_Z * d;
    }
    // ̧ͷ��ͷ���ƾֲ�����ϵ��X����ת��
    void Pitch(float rad)
    {
        // �ƾֲ�����ϵ��X����ת
        XMFLOAT4 saveTarget = mTarget;
        XMVECTOR newTarget = XMVector4Transform(XMLoadFloat4(&mTarget), XMMatrixRotationAxis(XMLoadFloat4(&mRightAxis), rad));
        XMStoreFloat4(&mTarget, newTarget);
        // ��ֹ���������
        //if (mTarget.y < -0.95f || mTarget.y > 0.95f) {
        //    mTarget = saveTarget;
        //}
    }
    // ����תͷ���ƾֲ�����ϵ��Y����ת��
    void Yaw(float rad)
    {
        // ��Y����ת
        XMVECTOR newTarget = XMVector4Transform(XMLoadFloat4(&mTarget), XMMatrixRotationY(rad));
        XMStoreFloat4(&mTarget, newTarget);
        // �ҷ���ҲҪ����
        XMVECTOR newRightAxis = XMVector4Transform(XMLoadFloat4(&mRightAxis), XMMatrixRotationY(rad));
        XMStoreFloat4(&mRightAxis, newRightAxis);
    }
};
FirstPersonCamera* m_pFirstPersonCamera = NULL; // ��һ�˳����ָ��

namespace AssimpModel
{
    //--------------------------------------------------------------------------------------
    // ����ģ�ͺ���
    //--------------------------------------------------------------------------------------
    void LoadModel(const aiScene* scene, Model* model)
    {
        for (int i = 0; i < scene->mNumMeshes; i++) {
            model->mNumFaces += scene->mMeshes[i]->mNumFaces; // ͳ����������
            model->mNumVertices += scene->mMeshes[i]->mNumVertices; // ͳ������mesh�Ķ����������ڿ����ڴ�ռ�
        }
        model->mNumIndices = 3 * model->mNumFaces; // ���������������������� = ���� * 3

        // ���������ڴ�
        model->vertices = new SimpleVertex[model->mNumVertices]; // ���ٶ���ռ�
        size_t count = 0; // ��¼����ƫ����
        std::vector<unsigned int> index_offset(1, 0);


        for (int i = 0; i < scene->mNumMeshes; i++) {
            for (int j = 0; j < scene->mMeshes[i]->mNumVertices; j++) {
                aiMaterial* material = scene->mMaterials[scene->mMeshes[i]->mMaterialIndex];
                aiColor3D color;
                // ��ȡmtl�ļ���������
                // material->Get(AI_MATKEY_COLOR_AMBIENT, color);
                material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
                // material->Get(AI_MATKEY_COLOR_SPECULAR, color);
                if (scene->mMeshes[i]->mTextureCoords[0]) {
                    model->vertices[count++] = {
                        XMFLOAT3(scene->mMeshes[i]->mVertices[j].x, scene->mMeshes[i]->mVertices[j].y, scene->mMeshes[i]->mVertices[j].z),
                        XMFLOAT3(scene->mMeshes[i]->mNormals[j].x, scene->mMeshes[i]->mNormals[j].y, scene->mMeshes[i]->mNormals[j].z),
                        XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
                        XMFLOAT2(scene->mMeshes[i]->mTextureCoords[0][0].x, scene->mMeshes[i]->mTextureCoords[0][0].y)
                    };
                }
                else {
                    model->vertices[count++] = {
                        XMFLOAT3(scene->mMeshes[i]->mVertices[j].x, scene->mMeshes[i]->mVertices[j].y, scene->mMeshes[i]->mVertices[j].z),
                        XMFLOAT3(scene->mMeshes[i]->mNormals[j].x, scene->mMeshes[i]->mNormals[j].y, scene->mMeshes[i]->mNormals[j].z),
                        XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
                        XMFLOAT2(0, 0)
                    };
                }

            }
            index_offset.push_back(count); // ƫ��������vector��
        }

        // ���������ڴ�
        model->indices = new WORD[model->mNumIndices]; // ���������ռ�
        count = 0; // ��¼����ƫ����
        for (int i = 0; i < scene->mNumMeshes; i++) {
            for (int j = 0; j < scene->mMeshes[i]->mNumFaces; j++) {
                aiFace face = scene->mMeshes[i]->mFaces[j];
                assert(face.mNumIndices == 3);
                for (int k = 0; k < face.mNumIndices; k++) {
                    int index = face.mIndices[k]; // indexֵ�����Լ�������������ݽṹ��
                    model->indices[count++] = index + index_offset[i]; // �ӵڶ���mesh��ʼ��Ҫ������������ڵ�һ��mesh��ƫ��
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------
    // ���ļ���������ģ��
    //--------------------------------------------------------------------------------------
    void LoadModelsFromFile(std::string filePath, std::vector<Model*>& models)
    {
        std::ifstream inFile;
        inFile.open(filePath, std::ios::in);
        if (inFile) {
            std::string modelName, modelPath;
            while (inFile >> modelName >> modelPath) {
                Model* model = new Model();
                model->mName = modelName;
                const aiScene* scene = aiImportFile(modelPath.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_Triangulate);
                LoadModel(scene, model);
                models.push_back(model);
            }
            inFile.close();
        }
    }

    //--------------------------------------------------------------------------------------
    // ��ģ��������Ⱦģ��
    //--------------------------------------------------------------------------------------
    void RenderModel(std::string modelName, ConstantBuffer &m_pConstantBuffer, XMMATRIX &m_pWorld)
    {
        int vertex_offset = 0;
        int index_offset = 0;
        for (int i = 0; i < models.size(); i++) {
            if (models[i]->mName == modelName) {
                m_pConstantBuffer.mWorld = XMMatrixTranspose(m_pWorld);
                g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);
                g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
                g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
                g_pImmediateContext->PSSetShader(g_pPixelShaders[1], NULL, 0);
                g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);
                g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
                g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
                g_pImmediateContext->DrawIndexed(models[i]->mNumIndices, index_offset, vertex_offset); // ������������ͼ��
            }
            index_offset += models[i]->mNumIndices;
            vertex_offset += models[i]->mNumVertices;
        }
    }
}

namespace Terrain
{
    //--------------------------------------------------------------------------------------
    // ���ظ߶�ͼ
    //--------------------------------------------------------------------------------------
    void LoadHeightMap(std::string filePath, std::string texturePath)
    {
        std::ifstream inFile;
        inFile.open(filePath, std::ios::binary); // �ö����Ƶķ�ʽ���ļ�
        std::vector<BYTE> inData(inFile.tellg()); // ��ģ�嶨��һ��vector<BYTE>���͵ı���inData����ʼ������ֵΪ��������ǰλ�ã�����������С
        inFile.seekg(std::ios::beg);  // ���ļ�ָ���ƶ����ļ��Ŀ�ͷ��׼����ȡ�߶���Ϣ
        inFile.read((char*)&inData[0], inData.size()); // ��ȡ�����߶���Ϣ
        inFile.close();

        std::vector<FLOAT> m_vHeightInfo;   // ���ڴ�Ÿ߶���Ϣ
        m_vHeightInfo.resize(inData.size()); // ��m_vHeightInfo�ߴ�ȡΪ�������ĳߴ�
        // ������������������inData�е�ֵ����m_vHeightInfo
        for (unsigned int i = 0; i < inData.size(); i++) {
            m_vHeightInfo[i] = inData[i];
        }
    }
}


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
    vertexData = { XMFLOAT3(0.0f, radius, 0.0f) };
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
            skyVertices[vIndex++] = { XMFLOAT3(x, y, z) };
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

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (FAILED(InitWindow(hInstance, nCmdShow)))
        return 0;

    if (FAILED(InitDevice()))
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Render();
        }
    }

    CleanupDevice();

    return (int)msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    g_hInst = hInstance;

    RECT rc = { 0, 0, 1280, 640 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    g_hWnd = CreateWindow(L"TutorialWindowClass", L"SimTown", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
        NULL);
    if (!g_hWnd)
        return E_FAIL;

    ShowWindow(g_hWnd, nCmdShow);

    m_pMouse->SetWindow(g_hWnd); // ������ô���
    m_pMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
    // Input::GetInstance()->Init(); // ��ʼ����������

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
    if (FAILED(hr))
    {
        if (pErrorBlob != NULL)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        if (pErrorBlob) pErrorBlob->Release();
        return hr;
    }
    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

//--------------------------------------------------------------------------------------
// ����Shader PSΪPixel Shader�����ƣ�pixel_shader_indexΪPixel Shader������
//--------------------------------------------------------------------------------------
HRESULT LoadShader(const char* PS, int pixel_shader_index)
{
    HRESULT hr = S_OK;

    // Compile the pixel shader
    ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile(L"Tutorial04.fx", PS, "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaders[pixel_shader_index]);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;

    return hr;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;
    
    RECT rc;
    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        return hr;

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
    if (FAILED(hr))
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
    if (FAILED(hr))
        return hr;

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports(1, &vp);

    // Compile the vertex shader
    ID3DBlob* pVSBlob = NULL;
    hr = CompileShaderFromFile(L"Tutorial04.fx", "VS", "vs_4_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &g_pVertexLayout);
    pVSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    char* shaders[5] = { "PS_Lambertian_Shading", "PS_Lambertian_Shading", "PS_Blinn_Phong_Shading", "PS_Lambertian_Shading", "PS_Texture_Mapping" };
    LoadShader(shaders[0], 0);
    LoadShader(shaders[1], 1);
    LoadShader(shaders[2], 2);
    LoadShader(shaders[3], 3);
    LoadShader(shaders[4], 4);

    D3D11_DEPTH_STENCIL_DESC dsDesc;
    // ����ʹ�����ֵһ�µ����ؽ����滻�����/ģ��״̬
    // ��״̬���ڻ�����պУ���Ϊ���ֵΪ1.0ʱĬ���޷�ͨ����Ȳ���
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = false;
    g_pd3dDevice->CreateDepthStencilState(&dsDesc, g_pDSS);
    //CreateDDSTextureFromFile( g_pd3dDevice, nullptr, L"Texture\\desertcube.dds", nullptr, &pTextureCubeSRV);

    m_pFirstPersonCamera = new FirstPersonCamera(Eye, At, Up, XMVector4Normalize(XMVector3Cross(Up, At))); // ������һ�˳����
    // ����ģ��
    AssimpModel::LoadModelsFromFile("models.txt", models);

    // ͳ���ܶ���������������
    for (auto x : models) {
        mNumVertices += x->mNumVertices;
        mNumIndices += x->mNumIndices;
    }
    // �����ܶ����������ָ���ڴ�
    SimpleVertex* vertices = new SimpleVertex[mNumVertices];
    WORD* indices = new WORD[mNumIndices];

    // ����ģ�����ݵ��ܶ�����������ڴ�
    int vertex_offset = 0;
    int index_offset = 0;
    for (int i = 0; i < models.size(); i++) {
        memcpy(vertices + vertex_offset, models[i]->vertices, sizeof(SimpleVertex) * models[i]->mNumVertices);
        memcpy(indices + index_offset, models[i]->indices, sizeof(WORD) * models[i]->mNumIndices);
        vertex_offset += models[i]->mNumVertices;
        index_offset += models[i]->mNumIndices;
    }

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * mNumVertices; // �����ܶ���ռ�õ��Դ�ռ�
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
        return hr;

    // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * mNumIndices; // ����������ռ�õ��Դ�ռ�
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
    if (FAILED(hr))
        return hr;

    // Set index buffer
    g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create the constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pConstantBuffer);
    if (FAILED(hr))
        return hr;

    // Load the Texture
    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"E:/Downloads/models/street_tile/StreetCorner.jpg", NULL, NULL, &g_pTextureRV, NULL);
    if (FAILED(hr))
        return hr;

    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
    if (FAILED(hr))
        return hr;

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 1000000.0f);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if (g_pImmediateContext) g_pImmediateContext->ClearState();

    for (int i = 0; i < g_pPixelShaders.size(); i++) {
        if (g_pPixelShaders[i]) g_pPixelShaders[i]->Release();
    }
    if (g_pSamplerLinear) g_pSamplerLinear->Release();
    if (g_pTextureRV) g_pTextureRV->Release();
    if (g_pConstantBuffer) g_pConstantBuffer->Release();
    if (g_pVertexBuffer) g_pVertexBuffer->Release();
    if (g_pIndexBuffer) g_pIndexBuffer->Release();
    if (g_pVertexLayout) g_pVertexLayout->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
    if (g_pPixelShader) g_pPixelShader->Release();
    if (g_pDepthStencil) g_pDepthStencil->Release();
    if (g_pDepthStencilView) g_pDepthStencilView->Release();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    // Input::GetInstance()->Listen(message, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); // ��������
    m_pKeyboard->ProcessMessage(message, wParam, lParam); // ��������
    m_pMouse->ProcessMessage(message, wParam, lParam);    // �������

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render()
{
    // Update our time
    static float t = 0.0f;
    if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float)XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();
        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;
        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }
    
    // �����ӽ�
    Eye = XMLoadFloat4(&m_pFirstPersonCamera->GetPosition());
    At = XMLoadFloat4(&m_pFirstPersonCamera->GetTarget());
    g_View = XMMatrixLookAtLH(Eye, Eye + At, Up);
    
    // ��������
    DirectX::Keyboard::State keyState = m_pKeyboard->GetState();
    if (keyState.IsKeyDown(DirectX::Keyboard::W))
    {
        m_pFirstPersonCamera->MoveForwardBack(2.0f);
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::S))
    {
        m_pFirstPersonCamera->MoveForwardBack(-2.0f);
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::A))
    {
        m_pFirstPersonCamera->MoveLeftRight(-0.1f);
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::D))
    {
        m_pFirstPersonCamera->MoveLeftRight(0.1f);
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::Escape))
    {
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
    }
    
    // �������
    DirectX::Mouse::State mouseState = m_pMouse->GetState();
    if (mouseState.positionMode == DirectX::Mouse::MODE_RELATIVE)
    {      
        m_pFirstPersonCamera->Pitch(mouseState.y * 0.005f);
        m_pFirstPersonCamera->Yaw(mouseState.x * 0.005f);
    }

    //
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    XMFLOAT4 vLightDirs[3] =
    {
        XMFLOAT4(-0.577f, 0.577f, -0.577f, 1.0f),
        XMFLOAT4(-0.577f, 0.577f, -0.577f, 1.0f),
        XMFLOAT4(0.0f, -0.707f, -0.707f, 1.0f)
    };
    XMFLOAT4 vLightColors[3] =
    {
        XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
        XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f),
        XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f)
    };

    XMFLOAT4 vCamera;
    XMStoreFloat4(&vCamera, Eye);
    float x_pos = 0;
    for (int i = 0; i < models.size(); i++, x_pos += 100.0f) {
        XMMATRIX mWorld = XMMatrixRotationY(t) * XMMatrixTranslation(x_pos, 0, 500.0f);
        cb.mView = XMMatrixTranspose(g_View);
        cb.mProjection = XMMatrixTranspose(g_Projection);

        cb.vLightDir[0] = vLightDirs[0];
        cb.vLightDir[1] = vLightDirs[1];
        cb.vLightDir[2] = vLightDirs[2];
        cb.vLightColor[0] = vLightColors[0];
        cb.vLightColor[1] = vLightColors[1];
        cb.vLightColor[2] = vLightColors[2];
        cb.vCamera = vCamera;

        AssimpModel::RenderModel(models[i]->mName, cb, mWorld);
    }

    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present(0, 0);
}
