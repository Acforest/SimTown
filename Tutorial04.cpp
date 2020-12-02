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
#include "resource.h"
#include "input.h"
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
    XMFLOAT4 vLightDir;     // ���߷���
    XMFLOAT4 vLightColor;   // ������ɫ
    XMFLOAT4 vCamera;       // ���λ��
    XMFLOAT4 vLookAt;       // �۲�λ��
};


struct Model
{
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
XMMATRIX                g_World;
XMMATRIX                g_View;
XMMATRIX                g_Projection;

std::vector<Model*> models; // ��ż��ص�����ģ��
std::vector<ID3D11PixelShader*> g_pPixelShaders(5, NULL);   // ��ż��ص����� Pixel Shader
std::unique_ptr<DirectX::Keyboard> m_pKeyboard = std::make_unique<DirectX::Keyboard>(); // ���̵���
std::unique_ptr<DirectX::Mouse> m_pMouse = std::make_unique<DirectX::Mouse>(); // ��굥��


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void Render();


//--------------------------------------------------------------------------------------
// ����ģ�ͺ���
//--------------------------------------------------------------------------------------
void LoadModel(const aiScene* scene, Model *model)
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
            //��ȡmtl�ļ���������
            //material->Get(AI_MATKEY_COLOR_AMBIENT, color);
            //mat.Ka = XMFLOAT4(color.r, color.g, color.b, 1.0);
            material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            //mat.Kd = XMFLOAT4(color.r, color.g, color.b, 1.0);
            //material->Get(AI_MATKEY_COLOR_SPECULAR, color);
            //mat.Ks = XMFLOAT4(color.r, color.g, color.b, 1.0);
            if (scene->mMeshes[i]->mTextureCoords[0]) {
                model->vertices[count++] = {
                    XMFLOAT3(scene->mMeshes[i]->mVertices[j].x, scene->mMeshes[i]->mVertices[j].y, scene->mMeshes[i]->mVertices[j].z),
                    XMFLOAT3(scene->mMeshes[i]->mNormals[j].x, scene->mMeshes[i]->mNormals[j].y, scene->mMeshes[i]->mNormals[j].z),
                    XMFLOAT4(color.r, color.g, color.b, 1.0f),
                    XMFLOAT2(scene->mMeshes[i]->mTextureCoords[0][0].x, scene->mMeshes[i]->mTextureCoords[0][0].y)
                };
            }
            else {
                model->vertices[count++] = {
                    XMFLOAT3(scene->mMeshes[i]->mVertices[j].x, scene->mMeshes[i]->mVertices[j].y, scene->mMeshes[i]->mVertices[j].z),
                    XMFLOAT3(scene->mMeshes[i]->mNormals[j].x, scene->mMeshes[i]->mNormals[j].y, scene->mMeshes[i]->mNormals[j].z),
                    XMFLOAT4(color.r, color.g, color.b, 1.0f),
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

    models.push_back(model);
}

//--------------------------------------------------------------------------------------
// ��ģ��������Ⱦģ��
//--------------------------------------------------------------------------------------
void RenderModel(int index)
{
    int vertex_offset = 0;
    int index_offset = 0;
    for (int i = 0; i < models.size(); i++) {
        if (i == index) {
            g_pImmediateContext->DrawIndexed(models[i]->mNumIndices, index_offset, vertex_offset); // ������������ͼ��
        }
        index_offset += models[i]->mNumIndices;
        vertex_offset += models[i]->mNumVertices;
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
    g_hWnd = CreateWindow(L"TutorialWindowClass", L"Direct3D 11 Tutorial 4: 3D Spaces", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
        NULL);
    if (!g_hWnd)
        return E_FAIL;

    ShowWindow(g_hWnd, nCmdShow);
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
    m_pMouse->SetWindow(g_hWnd); // ������ô���
    m_pMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
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


    // ����ģ��
    const aiScene* scene = aiImportFile("E://Downloads/models/buildings/hospital.obj", aiProcessPreset_TargetRealtime_MaxQuality);
    Model* model1 = new Model();

    Model* model2 = new Model();
    model2->mNumMeshes = 1;     // mesh��
    model2->mNumFaces = 2;      // ����mesh������
    model2->mNumVertices = 4;   // ����mesh�Ķ�����
    model2->mNumIndices = 6;    // ����mesh��������
    SimpleVertex vertices1[] = {
        { XMFLOAT3(-1.0, 0.0, 1.0), XMFLOAT3(0.0, 1.0, 0.0), XMFLOAT4(1.0, 1.0, 1.0, 1.0), XMFLOAT2(1.0, 1.0)},
        { XMFLOAT3(1.0, 0.0, 1.0), XMFLOAT3(0.0, 1.0, 0.0), XMFLOAT4(1.0, 1.0, 1.0, 1.0), XMFLOAT2(1.0, 1.0) },
        { XMFLOAT3(1.0, 0.0, -1.0), XMFLOAT3(0.0, 1.0, 0.0), XMFLOAT4(1.0, 1.0, 1.0, 1.0), XMFLOAT2(1.0, 1.0) },
        { XMFLOAT3(-1.0, 0.0, -1.0), XMFLOAT3(0.0, 1.0, 0.0), XMFLOAT4(1.0, 1.0, 1.0, 1.0), XMFLOAT2(1.0, 1.0) }
    }; // �����ڴ�
    WORD indices1[] = {
        0, 1, 2,
        2, 3, 0
    };          // �����ڴ�
    model2->vertices = vertices1;
    model2->indices = indices1;
    models.push_back(model2);
    
    LoadModel(scene, model1);
    

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
    g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f);

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

    // the view matrix
    static float eyeX = 0.0;
    static float eyeZ = -5.0;
    XMVECTOR Eye = XMVectorSet(eyeX, 2.0f, eyeZ, 0.0f);
    XMVECTOR At = XMVectorSet(eyeX, 2.0f, eyeZ + 10.0, 0.0f);
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    g_View = XMMatrixLookAtLH(Eye, At, Up);

    /*
    // ǰ����������
    if (Input::GetInstance()->IsKeyDown('W'))
    {
        eyeZ += 0.1;
    }
    if (Input::GetInstance()->IsKeyDown('S'))
    {
        eyeZ -= 0.1;
    }
    if (Input::GetInstance()->IsKeyDown('A'))
    {
        eyeX -= 0.1;
    }
    if (Input::GetInstance()->IsKeyDown('D'))
    {
        eyeX += 0.1;
    }

    if (Input::GetInstance()->IsMouseMove())
    {
        float mouseX = Input::GetInstance()->GetMouseX();
        float mouseY = Input::GetInstance()->GetMouseY();
        if (Input::GetInstance()->IsLMouseDown())
        {
            eyeZ -= 0.1;
        }
    }
    if (Input::GetInstance()->IsMouseMove())
    {
        float mouseX = Input::GetInstance()->GetMouseX();
        float mouseY = Input::GetInstance()->GetMouseY();
        if (Input::GetInstance()->IsLMouseDown())
        {
            float dx = XMConvertToRadians(0.25f * (mouseX - Input::GetInstance()->m_lastMousePos.x));
            float dy = XMConvertToRadians(0.25f * (mouseY - m_lastMousePos.y));

            OutputDebugString(L"left btn click");
            m_camera.Pitch(dy);
            m_camera.RotateY(dx);
        }
        m_lastMousePos.x = mouseX;
        m_lastMousePos.y = mouseY;
    }
    */
    DirectX::Keyboard::State keyState = m_pKeyboard->GetState();
    if (keyState.IsKeyDown(DirectX::Keyboard::W))
    {
        eyeZ += 0.5;
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::S))
    {
        eyeZ -= 0.5;
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::A))
    {
        eyeX -= 0.5;
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::D))
    {
        eyeX += 0.5;
    }
    DirectX::Mouse::State mouseState = m_pMouse->GetState();
    //DirectX::Mouse::State lastMouseState = m_MouseTracker.GetLastState();

    //
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    ConstantBuffer cb;
    int model_num = 1; // ��Ⱦ����ĸ���
    float x_pos = -6.0f; // ����߲����x����

    for (int i = 0; i < model_num; i++, x_pos += 2.0f) {
        g_World =  XMMatrixTranslation(x_pos, 0.0f, 0.0f);
        cb.mWorld = XMMatrixTranspose(g_World);
        cb.mView = XMMatrixTranspose(g_View);
        cb.mProjection = XMMatrixTranspose(g_Projection);

        cb.vLightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        cb.vLightDir = XMFLOAT4(0.0f, 1.0f, -3.0f, 1.0f);
        cb.vCamera = XMFLOAT4(0.0f, 2.0f, -5.0f, 1.0f);

        g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);
        g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
        g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
        g_pImmediateContext->PSSetShader(g_pPixelShaders[i], NULL, 0);
        g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);
        g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
        g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);

        RenderModel(1);
    }

    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present(0, 0);
}
