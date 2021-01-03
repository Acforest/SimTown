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
#include "FirstPersonCamera.h"
#include "FreeLookCamera.h"
#include "AssimpModel.h"
#include "Terrain.h"
#include "Helper.h"
#include "BoundingBox.h"


HINSTANCE                   g_hInst = NULL;
HWND                        g_hWnd = NULL;
D3D_DRIVER_TYPE             g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL           g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*               g_pd3dDevice = NULL;
ID3D11DeviceContext*        g_pImmediateContext = NULL;
IDXGISwapChain*             g_pSwapChain = NULL;
ID3D11RenderTargetView*     g_pRenderTargetView = NULL;
ID3D11Texture2D*            g_pDepthStencil = NULL;
ID3D11DepthStencilView*     g_pDepthStencilView = NULL;
ID3D11VertexShader*         g_pVertexShader = NULL;
ID3D11PixelShader*          g_pPixelShader = NULL;
ID3D11InputLayout*          g_pVertexLayout = NULL;
ID3D11Buffer*               g_pVertexBuffer = NULL;
ID3D11Buffer*               g_pIndexBuffer = NULL;
ID3D11Buffer*               g_pConstantBuffer = NULL;
ID3D11ShaderResourceView*   g_pTextureRV = NULL;
ID3D11SamplerState*         g_pSamplerLinear = NULL;
XMMATRIX                    g_World;
XMMATRIX                    g_View;
XMMATRIX                    g_Projection;
ConstantBuffer              cb;
ID3D11ShaderResourceView*   m_pTexture = NULL;
FirstPersonCamera*          m_pFirstPersonCamera = NULL; // 第一人称相机指针
FreeLookCamera*				m_pFreeLookCamera = NULL; // 第一人称相机指针
Terrain*					m_pTerrain; // 地形指针
ID3D11DepthStencilState*    g_pDSSNodepthWrite;  // 普通深度状态

ID3D11Buffer*				g_pSkyVertexBuffer = NULL;
ID3D11Buffer*				g_pSkyIndexBuffer = NULL;
ID3D11Buffer*				g_pSkyConstantBuffer = NULL;
ID3D11VertexShader*			g_pSkyVertexShader = NULL;
ID3D11ShaderResourceView*   g_pTextureSkySRV = NULL;
ID3D11DepthStencilState*    g_pDSSLessEqual;  // 天空盒深度状态
std::vector<ID3D11ShaderResourceView* >	g_pTextureCubeSRVs; // 天空盒SRV
std::vector<SimpleVertex>           skyVertices; // 天空盒顶点
std::vector<WORD>                   skyIndices;  // 天空盒索引

std::vector<Model*>                 models; // 存放加载的所有模型
std::vector<ID3D11PixelShader*>     g_pPixelShaders(6, NULL);   // 存放加载的所有 Pixel Shader
std::vector<XMMATRIX>               m_Worlds; // 存放所有加载的模型的世界矩阵
std::unique_ptr<DirectX::Keyboard>  m_pKeyboard = std::make_unique<DirectX::Keyboard>(); // 键盘单例
std::unique_ptr<DirectX::Mouse>     m_pMouse = std::make_unique<DirectX::Mouse>(); // 鼠标单例
DirectX::Mouse::ButtonStateTracker  m_MouseTracker; // 鼠标状态追踪

XMVECTOR Eye = XMVectorSet(0.0f, 10.0f, 0.0f, 0.0f);
XMVECTOR At = XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
int mNumVertices = 0; // 总顶点数
int mNumIndices = 0;  // 总索引数
bool mUsedCamera = 0;  // 使用的相机（0为第一人称相机，1为自由视角相机）
bool mTexMode = 0;  // 无纹理模型的着色（0为使用原色，1为使用木制贴图）
std::vector<AABB> AABBs;
std::vector<XMMATRIX> modelWorlds; // 存放模型所有世界矩阵
std::vector<std::string> mTextureNames;
std::vector<ID3D11ShaderResourceView* > mTextureRVs;

AABB treeAABB;


HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Render();