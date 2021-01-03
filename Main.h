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
FirstPersonCamera*          m_pFirstPersonCamera = NULL; // ��һ�˳����ָ��
FreeLookCamera*				m_pFreeLookCamera = NULL; // ��һ�˳����ָ��
Terrain*					m_pTerrain; // ����ָ��
ID3D11DepthStencilState*    g_pDSSNodepthWrite;  // ��ͨ���״̬

ID3D11Buffer*				g_pSkyVertexBuffer = NULL;
ID3D11Buffer*				g_pSkyIndexBuffer = NULL;
ID3D11Buffer*				g_pSkyConstantBuffer = NULL;
ID3D11VertexShader*			g_pSkyVertexShader = NULL;
ID3D11ShaderResourceView*   g_pTextureSkySRV = NULL;
ID3D11DepthStencilState*    g_pDSSLessEqual;  // ��պ����״̬
std::vector<ID3D11ShaderResourceView* >	g_pTextureCubeSRVs; // ��պ�SRV
std::vector<SimpleVertex>           skyVertices; // ��պж���
std::vector<WORD>                   skyIndices;  // ��պ�����

std::vector<Model*>                 models; // ��ż��ص�����ģ��
std::vector<ID3D11PixelShader*>     g_pPixelShaders(6, NULL);   // ��ż��ص����� Pixel Shader
std::vector<XMMATRIX>               m_Worlds; // ������м��ص�ģ�͵��������
std::unique_ptr<DirectX::Keyboard>  m_pKeyboard = std::make_unique<DirectX::Keyboard>(); // ���̵���
std::unique_ptr<DirectX::Mouse>     m_pMouse = std::make_unique<DirectX::Mouse>(); // ��굥��
DirectX::Mouse::ButtonStateTracker  m_MouseTracker; // ���״̬׷��

XMVECTOR Eye = XMVectorSet(0.0f, 10.0f, 0.0f, 0.0f);
XMVECTOR At = XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
int mNumVertices = 0; // �ܶ�����
int mNumIndices = 0;  // ��������
bool mUsedCamera = 0;  // ʹ�õ������0Ϊ��һ�˳������1Ϊ�����ӽ������
bool mTexMode = 0;  // ������ģ�͵���ɫ��0Ϊʹ��ԭɫ��1Ϊʹ��ľ����ͼ��
std::vector<AABB> AABBs;
std::vector<XMMATRIX> modelWorlds; // ���ģ�������������
std::vector<std::string> mTextureNames;
std::vector<ID3D11ShaderResourceView* > mTextureRVs;

AABB treeAABB;


HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Render();