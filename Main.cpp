#pragma once
#include "Main.h"

Terrain::Terrain()
{
    LoadHeightMap("models/terrain/smooth.raw");
    InitTerrain(5000, 5000, 50, 50, 1.0f);
    BuildBuffer();
    BuildSRVs(L"models/terrain/grass.png");
    BuildInputLayouts();
}

// 加载高度图
void Terrain::LoadHeightMap(std::string filePath)
{
    std::ifstream inFile;
    inFile.open(filePath, std::ios::binary); // 用二进制的方式打开文件
    inFile.seekg(0, std::ios::end); // 文件指针移动到末尾
    std::vector<BYTE> inData(inFile.tellg()); // 用模板定义一个vector<BYTE>类型的变量inData并初始化，其值为缓冲区当前位置，即缓冲区大小
    inFile.seekg(std::ios::beg);  // 将文件指针移动到文件的开头，准备读取高度信息
    inFile.read((char*)&inData[0], inData.size()); // 读取整个高度信息
    inFile.close();

    m_heightInfo.resize(inData.size()); // 将m_vHeightInfo尺寸取为缓冲区的尺寸
    // 遍历整个缓冲区，将inData中的值赋给m_vHeightInfo
    for (unsigned int i = 0; i < inData.size(); i++) {
        m_heightInfo[i] = inData[i];
    }
}

// 计算法线
void Terrain::ComputeNormal(SimpleVertex& v1, SimpleVertex& v2, SimpleVertex& v3, XMFLOAT3& normal)
{
    XMFLOAT3 f1(v2.Pos.x - v1.Pos.x, v2.Pos.y - v1.Pos.y, v2.Pos.z - v1.Pos.z);
    XMFLOAT3 f2(v3.Pos.x - v1.Pos.x, v3.Pos.y - v1.Pos.y, v3.Pos.z - v1.Pos.z);
    XMVECTOR vec1 = XMLoadFloat3(&f1);
    XMVECTOR vec2 = XMLoadFloat3(&f2);
    XMVECTOR temp = XMVector3Normalize(XMVector3Cross(vec1, vec2));
    XMStoreFloat3(&normal, temp);
}

void Terrain::InitTerrain(float width, float height, UINT m, UINT n, float scale)
{
    m_cellsPerRow = m;
    m_cellsPerCol = n;
    m_verticesPerRow = m + 1;
    m_verticesPerCol = n + 1;
    m_numsVertices = m_verticesPerRow * m_verticesPerCol;
    m_width = width;
    m_height = height;
    m_heightScale = scale;

    //得到缩放后的高度
    for (auto& item : m_heightInfo)
    {
        item *= m_heightScale;
    }

    //起始x z坐标
    float oX = -width * 0.5f;
    float oZ = height * 0.5f;
    //每一格坐标变化
    float dx = width / m;
    float dz = height / n;

    m_vertices.resize(m_numsVertices);
    //计算顶点
    for (UINT i = 0; i < m_verticesPerCol; ++i)
    {
        float tempZ = oZ - dz * i;
        for (UINT j = 0; j < m_verticesPerRow; ++j)
        {
            UINT index = m_verticesPerRow * i + j;
            m_vertices[index].Pos.x = oX + dx * j;
            m_vertices[index].Pos.y = m_heightInfo[index];
            m_vertices[index].Pos.z = tempZ;

            m_vertices[index].Texture = XMFLOAT2(dx * i, dx * j);
        }
    }

    //计算索引和法线
    //总格子数量:m * n
    //因此总索引数量: 6 * m * n
    UINT nIndices = m * n * 6;
    m_indices.resize(nIndices);
    UINT tmp = 0;
    for (UINT i = 0; i < n; ++i)
    {
        for (UINT j = 0; j < m; ++j)
        {
            m_indices[tmp] = i * m_verticesPerRow + j;
            m_indices[tmp + 1] = i * m_verticesPerRow + j + 1;
            m_indices[tmp + 2] = (i + 1) * m_verticesPerRow + j;

            //计算法线
            XMFLOAT3 temp;
            ComputeNormal(m_vertices[m_indices[tmp]], m_vertices[m_indices[tmp + 1]],
                m_vertices[m_indices[tmp + 2]], temp);
            m_vertices[m_indices[tmp]].Normal = temp;
            m_vertices[m_indices[tmp + 1]].Normal = temp;
            m_vertices[m_indices[tmp + 2]].Normal = temp;
            //m_vertices[m_indices[tmp]].Texture = XMFLOAT2(1, 1);
            //m_vertices[m_indices[tmp + 1]].Texture = XMFLOAT2(1, 1);
            //m_vertices[m_indices[tmp + 2]].Texture = XMFLOAT2(1, 1);

            m_indices[tmp + 3] = i * m_verticesPerRow + j + 1;
            m_indices[tmp + 4] = (i + 1) * m_verticesPerRow + j + 1;
            m_indices[tmp + 5] = (i + 1) * m_verticesPerRow + j;

            ComputeNormal(m_vertices[m_indices[tmp + 3]], m_vertices[m_indices[tmp + 4]],
                m_vertices[m_indices[tmp + 5]], temp);
            m_vertices[m_indices[tmp + 3]].Normal = temp;
            m_vertices[m_indices[tmp + 4]].Normal = temp;
            m_vertices[m_indices[tmp + 5]].Normal = temp;
            //m_vertices[m_indices[tmp + 3]].Texture = XMFLOAT2(1, 1);
            //m_vertices[m_indices[tmp + 4]].Texture = XMFLOAT2(1, 1);
            //m_vertices[m_indices[tmp + 5]].Texture = XMFLOAT2(1, 1);

            tmp += 6;
        }
    }
}

// 建立缓冲区
void Terrain::BuildBuffer()
{
    // 创建顶点缓冲区
    D3D11_BUFFER_DESC vertexDesc;
    ZeroMemory(&vertexDesc, sizeof(vertexDesc));
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.ByteWidth = sizeof(SimpleVertex) * m_numsVertices;
    vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;

    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = &m_vertices[0];
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;
    g_pd3dDevice->CreateBuffer(&vertexDesc, &vertexData, &m_pVertexBuffer);

    // 创建索引缓冲区
    D3D11_BUFFER_DESC indexDesc;
    ZeroMemory(&indexDesc, sizeof(indexDesc));
    indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexDesc.ByteWidth = sizeof(UINT) * m_indices.size();
    indexDesc.Usage = D3D11_USAGE_IMMUTABLE;

    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = &m_indices[0];
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;
    g_pd3dDevice->CreateBuffer(&indexDesc, &indexData, &m_pIndexBuffer);
}

// 建立纹理
void Terrain::BuildSRVs(const wchar_t* texturePath)
{
    // Load the Texture
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, texturePath, NULL, NULL, &m_pSRVTerrain, NULL);
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
    g_pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear);
}

// 建立输入布局
void Terrain::BuildInputLayouts()
{
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        //{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    UINT numLayoutElements = ARRAYSIZE(layout);
    ID3DBlob* pVSBlob = NULL;
    ID3D11InputLayout* m_pVertexLayout;
    CompileShaderFromFile(L"Tutorial04.fx", "VS", "vs_4_0", &pVSBlob);
    g_pd3dDevice->CreateInputLayout(layout, numLayoutElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &m_pVertexLayout);
    pVSBlob->Release();
}

void Terrain::Render(ConstantBuffer& cb, long vertex_offset, long index_offset)
{
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);
    g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->PSSetShader(g_pPixelShaders[4], NULL, 0);
    g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->PSSetShaderResources(1, 1, &m_pSRVTerrain);
    g_pImmediateContext->PSSetSamplers(0, 1, &m_pSamplerLinear);
    g_pImmediateContext->DrawIndexed(m_indices.size(), index_offset, vertex_offset);
}

namespace AssimpModel
{
    //--------------------------------------------------------------------------------------
    // 加载模型函数
    //--------------------------------------------------------------------------------------
    void LoadModel(const aiScene* scene, Model* model)
    {
        for (int i = 0; i < scene->mNumMeshes; i++) {
            model->mNumFaces += scene->mMeshes[i]->mNumFaces; // 统计所有面数
            model->mNumVertices += scene->mMeshes[i]->mNumVertices; // 统计所有mesh的顶点数，用于开辟内存空间
        }
        model->mNumIndices = 3 * model->mNumFaces; // 计算所有索引数，索引数 = 面数 * 3

        // 创建顶点内存
        model->vertices = new SimpleVertex[model->mNumVertices]; // 开辟顶点空间
        size_t count = 0; // 记录顶点偏移量
        std::vector<unsigned int> index_offset(1, 0);

        int meshVertexOffset = 0;
        int meshIndexOffset = 0;
        model->meshVertexOffset.resize(1);
        model->meshIndexOffset.resize(1);
        for (int i = 0; i < scene->mNumMeshes; i++) {
            aiMaterial* material = scene->mMaterials[scene->mMeshes[i]->mMaterialIndex];
            aiColor3D color;
            // 读取mtl文件顶点数据
            // material->Get(AI_MATKEY_COLOR_AMBIENT, color);
            material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            // material->Get(AI_MATKEY_COLOR_SPECULAR, color);
            model->mColors.push_back(XMFLOAT4(color.r, color.g, color.b, 1.0f));
            for (int j = 0; j < scene->mMeshes[i]->mNumVertices; j++) {
                if (scene->mMeshes[i]->mTextureCoords[0]) {
                    model->vertices[count++] = {
                        XMFLOAT3(scene->mMeshes[i]->mVertices[j].x, scene->mMeshes[i]->mVertices[j].y, scene->mMeshes[i]->mVertices[j].z),
                        XMFLOAT3(scene->mMeshes[i]->mNormals[j].x, scene->mMeshes[i]->mNormals[j].y, scene->mMeshes[i]->mNormals[j].z),
                        XMFLOAT4(color.r, color.g, color.b, 1.0f),
                        XMFLOAT2(scene->mMeshes[i]->mTextureCoords[0][j].x, scene->mMeshes[i]->mTextureCoords[0][j].y)
                    };
                }
                else {
                    model->vertices[count++] = {
                        XMFLOAT3(scene->mMeshes[i]->mVertices[j].x, scene->mMeshes[i]->mVertices[j].y, scene->mMeshes[i]->mVertices[j].z),
                        XMFLOAT3(scene->mMeshes[i]->mNormals[j].x, scene->mMeshes[i]->mNormals[j].y, scene->mMeshes[i]->mNormals[j].z),
                        XMFLOAT4(color.r, color.g, color.b, 1.0f),
                        XMFLOAT2(1.0f, 1.0f)
                    };
                }

            }
            meshVertexOffset += scene->mMeshes[i]->mNumVertices;
            model->meshVertexOffset.push_back(meshVertexOffset);
            index_offset.push_back(count); // 偏移量放入vector中
        }

        // 创建索引内存
        model->indices = new WORD[model->mNumIndices]; // 开辟索引空间
        count = 0; // 记录顶点偏移量
        for (int i = 0; i < scene->mNumMeshes; i++) {
            for (int j = 0; j < scene->mMeshes[i]->mNumFaces; j++) {
                aiFace face = scene->mMeshes[i]->mFaces[j];
                assert(face.mNumIndices == 3);
                for (int k = 0; k < face.mNumIndices; k++) {
                    int index = face.mIndices[k]; // index值存入自己定义的索引数据结构中
                    model->indices[count++] = index + index_offset[i]; // 从第二个mesh开始，要考虑索引相对于第一个mesh的偏移
                }
            }
            meshIndexOffset += 3 * scene->mMeshes[i]->mNumFaces;
            model->meshIndexOffset.push_back(meshIndexOffset);

        }
    }

    //--------------------------------------------------------------------------------------
    // 从文件读入所有模型
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
    // 按模型名称渲染模型
    //--------------------------------------------------------------------------------------
    void RenderModel(std::string modelName, ConstantBuffer& m_pConstantBuffer, XMMATRIX& m_pWorld)
    {
        int vertex_offset = 0;
        int index_offset = 0;
        // Set the input layout
        g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
        // 设置模型顶点缓冲区
        UINT stride = sizeof(SimpleVertex);
        UINT offset = 0;
        g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
        // 设置模型索引缓冲区
        g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
        for (int i = 0; i < models.size(); i++) {
            if (models[i]->mName == modelName) {
                for (int j = 0; j < models[i]->meshIndexOffset.size(); j++) {
                    m_pConstantBuffer.mWorld = XMMatrixTranspose(m_pWorld);
                    m_pConstantBuffer.mColor = models[i]->mColors[0];
                    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);
                    g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
                    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
                    g_pImmediateContext->PSSetShader(g_pPixelShaders[4], NULL, 0);
                    g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);
                    g_pImmediateContext->PSSetShaderResources(1, 1, &g_pTextureRV);
                    g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
                    g_pImmediateContext->DrawIndexed(models[i]->meshIndexOffset[j], index_offset, vertex_offset); // 按索引数绘制图形
                }
            }
            index_offset += models[i]->mNumIndices;
            vertex_offset += models[i]->mNumVertices;
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

    m_pMouse->SetWindow(g_hWnd); // 鼠标设置窗口
    m_pMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
    // Input::GetInstance()->Init(); // 初始化键鼠输入

    return S_OK;
}


//--------------------------------------------------------------------------------------
// 加载Shader PS为Pixel Shader的名称，pixel_shader_index为Pixel Shader的索引
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

    // 加载Shader
    char* shaders[5] = { "PS_Lambertian_Shading", "PS_Lambertian_Shading", "PS_Blinn_Phong_Shading", "PS_Lambertian_Shading", "PS_Texture_Mapping" };
    LoadShader(shaders[0], 0);
    LoadShader(shaders[1], 1);
    LoadShader(shaders[2], 2);
    LoadShader(shaders[3], 3);
    LoadShader(shaders[4], 4);
    // 创建第一人称相机
    m_pFirstPersonCamera = new FirstPersonCamera(Eye, At, Up, XMVector4Normalize(XMVector3Cross(Up, At)));
    // 加载模型
    AssimpModel::LoadModelsFromFile("models.txt", models);

    // 统计总顶点数和总索引数
    for (auto x : models) {
        mNumVertices += x->mNumVertices;
        mNumIndices += x->mNumIndices;
    }
    m_pTerrain = new Terrain(); // 创建地形
    mNumVertices += m_pTerrain->m_vertices.size();
    mNumIndices += m_pTerrain->m_indices.size();

    // 开辟总顶点和总索引指针内存
    SimpleVertex* vertices = new SimpleVertex[mNumVertices];
    WORD* indices = new WORD[mNumIndices];

    // 拷贝模型数据到总顶点和总索引内存
    long long vertex_offset = 0;
    long long index_offset = 0;
    for (int i = 0; i < models.size(); i++) {
        memcpy(vertices + vertex_offset, models[i]->vertices, sizeof(SimpleVertex) * models[i]->mNumVertices);
        memcpy(indices + index_offset, models[i]->indices, sizeof(WORD) * models[i]->mNumIndices);
        vertex_offset += models[i]->mNumVertices;
        index_offset += models[i]->mNumIndices;
    }
    memcpy(vertices + vertex_offset, &m_pTerrain->m_vertices[0], sizeof(SimpleVertex) * m_pTerrain->m_vertices.size());
    memcpy(indices + index_offset, &m_pTerrain->m_indices[0], sizeof(WORD) * m_pTerrain->m_indices.size());
    m_pTerrain->vertex_offset = vertex_offset;
    m_pTerrain->index_offset = index_offset;

    D3D11_SUBRESOURCE_DATA InitData;
    /*
    // 天空盒顶点缓冲区描述
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(SimpleVertex) * skyVertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    // 创建天空盒顶点缓冲区
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = &skyVertices[0];
    g_pd3dDevice->CreateBuffer(&vbd, &InitData, &g_pSkyVertexBuffer);
    // 天空盒索引缓冲区描述
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(WORD) * skyIndices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    // 创建天空盒索引缓冲区
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = &skyIndices[0];
    g_pd3dDevice->CreateBuffer(&ibd, &InitData, &g_pSkyIndexBuffer);
    // 常量缓冲区描述
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    g_pd3dDevice->CreateBuffer(&cbd, nullptr, &g_pSkyConstantBuffer);
    // 允许使用深度值一致的像素进行替换的深度/模板状态
    // 该状态用于绘制天空盒，因为深度值为1.0时默认无法通过深度测试
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = false;
    g_pd3dDevice->CreateDepthStencilState(&dsDesc, g_pDSS);
    g_pTextureCubeSRVs.resize(6);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/sunset_posX.bmp", NULL, NULL, &g_pTextureCubeSRVs[0], NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/sunset_posY.bmp", NULL, NULL, &g_pTextureCubeSRVs[1], NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/sunset_posZ.bmp", NULL, NULL, &g_pTextureCubeSRVs[2], NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/sunset_negX.bmp", NULL, NULL, &g_pTextureCubeSRVs[3], NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/sunset_negY.bmp", NULL, NULL, &g_pTextureCubeSRVs[4], NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/sunset_negZ.bmp", NULL, NULL, &g_pTextureCubeSRVs[5], NULL);
    // 编译天空盒顶点Shader
    pVSBlob = NULL;
    hr = CompileShaderFromFile(L"Tutorial04.fx", "Sky_VS", "vs_4_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }
    // 创建天空盒顶点Shader
    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pSkyVertexShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }*/

    // 模型顶点缓冲区描述
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * mNumVertices; // 设置总顶点占用的显存空间
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    // 创建模型顶点缓冲区
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
        return hr;
    // 模型索引缓冲区描述
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * mNumIndices; // 设置总索引占用的显存空间
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
    if (FAILED(hr))
        return hr;

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
    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/tile_wood.jpg", NULL, NULL, &g_pTextureRV, NULL);
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
    // Input::GetInstance()->Listen(message, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); // 监听键鼠
    m_pKeyboard->ProcessMessage(message, wParam, lParam); // 监听键盘
    m_pMouse->ProcessMessage(message, wParam, lParam);    // 监听鼠标

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
    
    // 更新视角
    Eye = XMLoadFloat4(&m_pFirstPersonCamera->GetPosition());
    At = XMLoadFloat4(&m_pFirstPersonCamera->GetTarget());
    g_View = XMMatrixLookAtLH(Eye, Eye + At, Up);
    
    // 监听键盘
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
        m_pFirstPersonCamera->MoveLeftRight(-0.5f);
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::D))
    {
        m_pFirstPersonCamera->MoveLeftRight(0.5f);
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::Escape))
    {
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
    }
    
    // 监听鼠标
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
        XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f),
        XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
        XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f)
    };

    XMFLOAT4 vCamera;
    XMStoreFloat4(&vCamera, Eye);
    float x_pos = 0;
    for (int i = 0; i < models.size(); i++, x_pos += 100.0f) {
        XMMATRIX mWorld = XMMatrixTranslation(x_pos, 0, 500.0f);
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
    ConstantBuffer tcb;
    tcb.mWorld = XMMatrixTranslation(0, 0, 0) * XMMatrixScaling(1, 0, 1);
    tcb.mView = XMMatrixTranspose(g_View);
    tcb.mProjection = XMMatrixTranspose(g_Projection);
    tcb.vLightDir[0] = vLightDirs[0];
    tcb.vLightDir[1] = vLightDirs[1];
    tcb.vLightDir[2] = vLightDirs[2];
    tcb.vLightColor[0] = vLightColors[0];
    tcb.vLightColor[1] = vLightColors[1];
    tcb.vLightColor[2] = vLightColors[2];
    m_pTerrain->Render(tcb, m_pTerrain->vertex_offset, m_pTerrain->index_offset);
    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present(0, 0);
}
