#pragma once
#pragma comment(lib, "lib/assimp-vc140-mt.dll")
#include "Main.h"
#include <Importer.hpp>

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
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
    g_pImmediateContext->OMSetDepthStencilState(g_pDSSNodepthWrite, 0);
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
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
        int mNumVertices = 0;
        int mNumFaces = 0;
        int mNumIndices = 0;
        for (int i = 0; i < scene->mNumMeshes; i++) {
            mNumFaces += scene->mMeshes[i]->mNumFaces; // 统计所有面数
            mNumVertices += scene->mMeshes[i]->mNumVertices; // 统计所有mesh的顶点数，用于开辟内存空间
        }
        mNumIndices = 3 * mNumFaces; // 计算所有索引数，索引数 = 面数 * 3

        // 创建顶点内存
        model->vertices.resize(mNumVertices); // 开辟顶点空间
        size_t count = 0; // 记录顶点偏移量
        std::vector<unsigned int> index_offset(1, 0);

        int meshVertexOffset = 0;
        int meshIndexOffset = 0;
        model->meshVertexOffset.resize(1);
        model->meshIndexOffset.resize(1);
        for (int i = 0; i < scene->mNumMeshes; i++) {
            aiMaterial* material = scene->mMaterials[scene->mMeshes[i]->mMaterialIndex];
            aiColor3D color;
            // 加载颜色
            // material->Get(AI_MATKEY_COLOR_AMBIENT, color);
            material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            // material->Get(AI_MATKEY_COLOR_SPECULAR, color);
            model->mColors.push_back(XMFLOAT4(color.r, color.g, color.b, 1.0f));
            // 加载纹理
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString Path;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL);
                model->mTextureNames.push_back(Path.C_Str());
            }
            for (int j = 0; j < scene->mMeshes[i]->mNumVertices; j++) {
                if (scene->mMeshes[i]->HasTextureCoords(0) && scene->mMeshes[i]->HasNormals()) {
                    model->vertices[count++] = {
                        XMFLOAT3(scene->mMeshes[i]->mVertices[j].x, scene->mMeshes[i]->mVertices[j].y, scene->mMeshes[i]->mVertices[j].z),
                        XMFLOAT3(scene->mMeshes[i]->mNormals[j].x, scene->mMeshes[i]->mNormals[j].y, scene->mMeshes[i]->mNormals[j].z),
                        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
                        XMFLOAT2(scene->mMeshes[i]->mTextureCoords[0][j].x, scene->mMeshes[i]->mTextureCoords[0][j].y)
                    };
                }
                else if (scene->mMeshes[i]->HasTextureCoords(0) && !scene->mMeshes[i]->HasNormals()) {
                    model->vertices[count++] = {
                        XMFLOAT3(scene->mMeshes[i]->mVertices[j].x, scene->mMeshes[i]->mVertices[j].y, scene->mMeshes[i]->mVertices[j].z),
                        XMFLOAT3(0.0f, 0.0f, 1.0f),
                        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
                        XMFLOAT2(scene->mMeshes[i]->mTextureCoords[0][j].x, scene->mMeshes[i]->mTextureCoords[0][j].y)
                    };
                }
                else if (!scene->mMeshes[i]->HasTextureCoords(0) && scene->mMeshes[i]->HasNormals()) {
                    model->vertices[count++] = {
                        XMFLOAT3(scene->mMeshes[i]->mVertices[j].x, scene->mMeshes[i]->mVertices[j].y, scene->mMeshes[i]->mVertices[j].z),
                        XMFLOAT3(scene->mMeshes[i]->mNormals[j].x, scene->mMeshes[i]->mNormals[j].y, scene->mMeshes[i]->mNormals[j].z),
                        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
                        XMFLOAT2(1.0f, 1.0f)
                    };
                }
            }
            meshVertexOffset += scene->mMeshes[i]->mNumVertices;
            model->meshVertexOffset.push_back(meshVertexOffset);
            index_offset.push_back(count); // 偏移量放入vector中
        }


        // 创建索引内存
        model->indices.resize(mNumIndices); // 开辟索引空间
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

        D3D11_SUBRESOURCE_DATA InitData;
        // 模型顶点缓冲区描述
        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(SimpleVertex) * model->vertices.size(); // 设置总顶点占用的显存空间
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        // 创建模型顶点缓冲区
        ZeroMemory(&InitData, sizeof(InitData));
        InitData.pSysMem = &model->vertices[0];
        g_pd3dDevice->CreateBuffer(&bd, &InitData, &model->mVertexBuffer);
        // 模型索引缓冲区描述
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(WORD) * model->indices.size(); // 设置总索引占用的显存空间
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;
        InitData.pSysMem = &model->indices[0];
        g_pd3dDevice->CreateBuffer(&bd, &InitData, &model->mIndexBuffer);
        // 模型常量缓冲区描述
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(ConstantBuffer);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0;
        g_pd3dDevice->CreateBuffer(&bd, NULL, &model->mConstantBuffer);
    }

    //--------------------------------------------------------------------------------------
    // 从文件读入所有模型
    //--------------------------------------------------------------------------------------
    void LoadModelsFromFile(std::string filePath, std::vector<Model*>& models)
    {
        std::ifstream inFile;
        inFile.open(filePath, std::ios::in);
        Assimp::Importer importer;
        if (inFile) {
            std::string modelName, modelPath;
            while (inFile >> modelName >> modelPath) {
                Model* model = new Model();
                model->mName = modelName;
                const aiScene* scene = importer.ReadFile(modelPath.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded |
                    aiProcess_Triangulate | aiProcess_FixInfacingNormals);
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
        g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
        g_pImmediateContext->OMSetDepthStencilState(g_pDSSNodepthWrite, 0);
        for (int i = 0; i < models.size(); i++) {
            if (models[i]->mName == modelName) {
                UINT stride = sizeof(SimpleVertex);
                UINT offset = 0;
                g_pImmediateContext->IASetVertexBuffers(0, 1, &models[i]->mVertexBuffer, &stride, &offset);
                g_pImmediateContext->IASetIndexBuffer(models[i]->mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
                for (int j = 0; j < models[i]->meshIndexOffset.size() - 1; j++) {
                    m_pConstantBuffer.mWorld = XMMatrixTranspose(m_pWorld);
                    // 非地形
                    if (i != 0) {
                        m_pConstantBuffer.mColor = models[i]->mColors[j];
                    }
                    // 地形
                    else {
                        m_pConstantBuffer.mColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
                    }
                    g_pImmediateContext->UpdateSubresource(models[i]->mConstantBuffer, 0, NULL, &cb, 0, 0);
                    g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
                    g_pImmediateContext->VSSetConstantBuffers(0, 1, &models[i]->mConstantBuffer);
                    // 无贴图
                    if (models[i]->mTextureNames.empty()) {
                        // 无贴图模式
                        if (mTexMode == 0) {
                            g_pImmediateContext->PSSetShader(g_pPixelShaders[0], NULL, 0);
                            g_pImmediateContext->PSSetConstantBuffers(0, 1, &models[i]->mConstantBuffer);
                        }
                        // 贴木制贴图
                        else {
                            g_pImmediateContext->PSSetShader(g_pPixelShaders[4], NULL, 0);
                            g_pImmediateContext->PSSetConstantBuffers(0, 1, &models[i]->mConstantBuffer);
                            g_pImmediateContext->PSSetShaderResources(1, 1, &g_pTextureRV);
                        }
                    }
                    // 有贴图
                    else {
                        bool isFound = false;
                        for (int k = 0; k < mTextureNames.size(); k++) {
                            if (mTextureNames[k] == models[i]->mTextureNames[j]) {
                                isFound = true;
                                g_pImmediateContext->PSSetShader(g_pPixelShaders[4], NULL, 0);
                                g_pImmediateContext->PSSetConstantBuffers(0, 1, &models[i]->mConstantBuffer);
                                g_pImmediateContext->PSSetShaderResources(1, 1, &mTextureRVs[k]);
                                break;
                            }
                        }
                        if (isFound == false) {
                            g_pImmediateContext->PSSetShader(g_pPixelShaders[4], NULL, 0);
                            g_pImmediateContext->PSSetConstantBuffers(0, 1, &models[i]->mConstantBuffer);
                            g_pImmediateContext->PSSetShaderResources(1, 1, &g_pTextureRV);
                        }
                    }
                    g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
                    g_pImmediateContext->DrawIndexed(models[i]->meshIndexOffset[j], index_offset, vertex_offset); // 按索引数绘制图形
                }
            }
            index_offset += models[i]->indices.size();
            vertex_offset += models[i]->vertices.size();
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
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &g_pVertexLayout);
    pVSBlob->Release();
    if (FAILED(hr))
        return hr;

    // 加载Shader
    char* shaders[] = { "PS_Ambient_Shading", "PS_Lambertian_Shading", "PS_Blinn_Phong_Shading", "PS_Toon_Shading", "PS_Texture_Mapping", "PS_Sky_Texture" };
    LoadShader(shaders[0], 0);
    LoadShader(shaders[1], 1);
    LoadShader(shaders[2], 2);
    LoadShader(shaders[3], 3);
    LoadShader(shaders[4], 4);
    LoadShader(shaders[5], 5);
    // 创建第一人称相机
    m_pFirstPersonCamera = new FirstPersonCamera(Eye, At, Up, XMVector4Normalize(XMVector3Cross(Up, At)));
    // 创建自由视角相机
    m_pFreeLookCamera = new FreeLookCamera(Eye, At, Up, XMVector4Normalize(XMVector3Cross(Up, At)));
    // 加载模型
    AssimpModel::LoadModelsFromFile("models.txt", models);

    modelWorlds.push_back(XMMatrixScaling(50.0f, 50.0f, 50.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f) * XMMatrixRotationY(0.0f)); // terrain
    modelWorlds.push_back(XMMatrixScaling(5.0f, 5.0f, 5.0f) * XMMatrixTranslation(-500.0f, 0.0f, 1000.0f) * XMMatrixRotationY(0.0f));// highschool
    modelWorlds.push_back(XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(1000.0f, 0.0f, 500.0f) * XMMatrixRotationY(0.0f)); // hospital
    modelWorlds.push_back(XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(1000.0f, 0.0f, 0.0f) * XMMatrixRotationY(0.0f)); // house1
    modelWorlds.push_back(XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(1000.0f, 0.0f, -500.0f) * XMMatrixRotationY(0.0f)); // house2
    modelWorlds.push_back(XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(1000.0f, 0.0f, -1000.0f) * XMMatrixRotationY(0.0f)); // house3
    modelWorlds.push_back(XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(-500.0f, 0.0f, 300.0f) * XMMatrixRotationY(0.0f)); // house4
    modelWorlds.push_back(XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(-500.0f, 0.0f, -500.0f) * XMMatrixRotationY(0.0f)); // house5
    modelWorlds.push_back(XMMatrixScaling(10.0f, 10.0f, 10.0f) * XMMatrixTranslation(-500.0f, 0.0f, 500.0f) * XMMatrixRotationY(0.0f)); // park
    modelWorlds.push_back(XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(500.0f, 0.0f, 300.0f) * XMMatrixRotationY(0.0f)); // police
    modelWorlds.push_back(XMMatrixScaling(5.0f, 5.0f, 5.0f) * XMMatrixTranslation(500.0f, 0.0f, 100.0f) * XMMatrixRotationY(0.0f)); // shop
    treeAABB.SetAABB(XMMatrixScaling(5.0f, 5.0f, 5.0f) * XMMatrixTranslation(500.0f, 0.0f, 100.0f) * XMMatrixRotationY(0.0f), models[10]);
    modelWorlds.push_back(XMMatrixScaling(10.0f, 10.0f, 10.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f) * XMMatrixRotationY(0.0f)); // car
    modelWorlds.push_back(XMMatrixScaling(5.0f, 5.0f, 5.0f) * XMMatrixTranslation(0.0f, 0.0f, -700.0f) * XMMatrixRotationY(0.0f)); // tree
    modelWorlds.push_back(XMMatrixScaling(5.0f, 5.0f, 5.0f) * XMMatrixTranslation(0.0f, 0.0f, -900.0f) * XMMatrixRotationY(0.0f)); // tree2
    

    ID3D11ShaderResourceView *SRV1, *SRV2, *SRV3, *SRV4, *SRV5, *SRV6, *SRV7, *SRV8, *SRV9, *SRV10, *SRV11, *SRV12, *SRV13;
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/grass.png", NULL, NULL, &SRV1, NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/car_texture.png", NULL, NULL, &SRV2, NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/StreetCorner.jpg", NULL, NULL, &SRV3, NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/StreetCrossing.jpg", NULL, NULL, &SRV4, NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/StreetEnd.jpg", NULL, NULL, &SRV5, NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/StreetLine.jpg", NULL, NULL, &SRV6, NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/StreetT.jpg", NULL, NULL, &SRV7, NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/tile_wood.jpg", NULL, NULL, &SRV8, NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/tree_texture.jpg", NULL, NULL, &SRV9, NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/M61501.dds", NULL, NULL, &SRV10, NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/M61501_n.dds", NULL, NULL, &SRV11, NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/M61501_s.dds", NULL, NULL, &SRV12, NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/male_diffuse_white_med-facial.jpg", NULL, NULL, &SRV13, NULL);
    mTextureRVs.push_back(SRV1); mTextureNames.push_back("grass.png");
    mTextureRVs.push_back(SRV2); mTextureNames.push_back("car_texture.png");
    mTextureRVs.push_back(SRV3); mTextureNames.push_back("StreetCorner.jpg");
    mTextureRVs.push_back(SRV4); mTextureNames.push_back("StreetCrossing.jpg");
    mTextureRVs.push_back(SRV5); mTextureNames.push_back("StreetEnd.jpg");
    mTextureRVs.push_back(SRV6); mTextureNames.push_back("StreetLine.jpg");
    mTextureRVs.push_back(SRV7); mTextureNames.push_back("StreetT.jpg");
    mTextureRVs.push_back(SRV8); mTextureNames.push_back("tile_wood.jpg");
    mTextureRVs.push_back(SRV9); mTextureNames.push_back("tree_texture.jpg");
    mTextureRVs.push_back(SRV10); mTextureNames.push_back("M61501.dds");
    mTextureRVs.push_back(SRV11); mTextureNames.push_back("M61501_n.dds");
    mTextureRVs.push_back(SRV12); mTextureNames.push_back("M61501_s.dds");
    mTextureRVs.push_back(SRV13); mTextureNames.push_back("male_diffuse_white_med-facial.jpg");
    
    for (int i = 0; i < models.size(); i++) {
        AABB aabb;
        aabb.SetAABB(modelWorlds[i], models[i]);
        AABBs.push_back(aabb);
    }

    //memcpy(vertices + vertex_offset, &m_pTerrain->m_vertices[0], sizeof(SimpleVertex) * m_pTerrain->m_vertices.size());
    //memcpy(indices + index_offset, &m_pTerrain->m_indices[0], sizeof(WORD) * m_pTerrain->m_indices.size());
    //m_pTerrain->vertex_offset = vertex_offset;
    //m_pTerrain->index_offset = index_offset;

    int levels = 50, slices = 50, radius = 100;
    // 创建天空球的顶点数据和索引数据（逆时针绘制）
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
    // 放入顶端点
    vertexData = { XMFLOAT3(0.0f, radius, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };
    skyVertices[vIndex++] = vertexData;
    for (UINT i = 1; i < levels; ++i)
    {
        phi = per_phi * i;
        // 需要slices + 1个顶点是因为 起点和终点需为同一点，但纹理坐标值不一致
        for (UINT j = 0; j <= slices; ++j)
        {
            theta = per_theta * j;
            x = radius * sinf(phi) * cosf(theta);
            y = radius * cosf(phi);
            z = radius * sinf(phi) * sinf(theta);
            // 计算出局部坐标、法向量、Tangent向量和纹理坐标
            XMFLOAT3 pos = XMFLOAT3(x, y, z), normal;
            XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&pos)));
            vertexData = { pos, normal, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(theta / XM_2PI, phi / XM_PI) };
            skyVertices[vIndex++] = { XMFLOAT3(x, y, z),  };
        }
    }
    // 放入底端点
    vertexData = { XMFLOAT3(0.0f, -radius, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) };
    skyVertices[vIndex++] = vertexData;
    // 放入索引
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

    cubeVertexVec.resize(24);
    float w2 = (treeAABB.MaxPos.x - treeAABB.MinPos.x) / 2, h2 = (treeAABB.MaxPos.y - treeAABB.MinPos.y) / 2, d2 = (treeAABB.MaxPos.z - treeAABB.MinPos.z) / 2;

    // 右面(+X面)
    cubeVertexVec[0].Pos = XMFLOAT3(w2, -h2, -d2);
    cubeVertexVec[1].Pos = XMFLOAT3(w2, h2, -d2);
    cubeVertexVec[2].Pos = XMFLOAT3(w2, h2, d2);
    cubeVertexVec[3].Pos = XMFLOAT3(w2, -h2, d2);
    // 左面(-X面)
    cubeVertexVec[4].Pos = XMFLOAT3(-w2, -h2, d2);
    cubeVertexVec[5].Pos = XMFLOAT3(-w2, h2, d2);
    cubeVertexVec[6].Pos = XMFLOAT3(-w2, h2, -d2);
    cubeVertexVec[7].Pos = XMFLOAT3(-w2, -h2, -d2);
    // 顶面(+Y面)
    cubeVertexVec[8].Pos = XMFLOAT3(-w2, h2, -d2);
    cubeVertexVec[9].Pos = XMFLOAT3(-w2, h2, d2);
    cubeVertexVec[10].Pos = XMFLOAT3(w2, h2, d2);
    cubeVertexVec[11].Pos = XMFLOAT3(w2, h2, -d2);
    // 底面(-Y面)
    cubeVertexVec[12].Pos = XMFLOAT3(w2, -h2, -d2);
    cubeVertexVec[13].Pos = XMFLOAT3(w2, -h2, d2);
    cubeVertexVec[14].Pos = XMFLOAT3(-w2, -h2, d2);
    cubeVertexVec[15].Pos = XMFLOAT3(-w2, -h2, -d2);
    // 背面(+Z面)
    cubeVertexVec[16].Pos = XMFLOAT3(w2, -h2, d2);
    cubeVertexVec[17].Pos = XMFLOAT3(w2, h2, d2);
    cubeVertexVec[18].Pos = XMFLOAT3(-w2, h2, d2);
    cubeVertexVec[19].Pos = XMFLOAT3(-w2, -h2, d2);
    // 正面(-Z面)
    cubeVertexVec[20].Pos = XMFLOAT3(-w2, -h2, -d2);
    cubeVertexVec[21].Pos = XMFLOAT3(-w2, h2, -d2);
    cubeVertexVec[22].Pos = XMFLOAT3(w2, h2, -d2);
    cubeVertexVec[23].Pos = XMFLOAT3(w2, -h2, -d2);

    for (UINT i = 0; i < 4; ++i)
    {
        // 右面(+X面)
        cubeVertexVec[i].Normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
        cubeVertexVec[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        // 左面(-X面)
        cubeVertexVec[i + 4].Normal = XMFLOAT3(-1.0f, 0.0f, 0.0f);
        cubeVertexVec[i + 4].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        // 顶面(+Y面)
        cubeVertexVec[i + 8].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
        cubeVertexVec[i + 8].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        // 底面(-Y面)
        cubeVertexVec[i + 12].Normal = XMFLOAT3(0.0f, -1.0f, 0.0f);
        cubeVertexVec[i + 12].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        // 背面(+Z面)
        cubeVertexVec[i + 16].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
        cubeVertexVec[i + 16].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        // 正面(-Z面)
        cubeVertexVec[i + 20].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
        cubeVertexVec[i + 20].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    for (UINT i = 0; i < 6; ++i)
    {
        cubeVertexVec[i * 4].Texture = XMFLOAT2(0.0f, 1.0f);
        cubeVertexVec[i * 4 + 1].Texture = XMFLOAT2(0.0f, 0.0f);
        cubeVertexVec[i * 4 + 2].Texture = XMFLOAT2(1.0f, 0.0f);
        cubeVertexVec[i * 4 + 3].Texture = XMFLOAT2(1.0f, 1.0f);
    }
    cubeIndexVec = {
        0, 1, 2, 2, 3, 0,        // 右面(+X面)
        4, 5, 6, 6, 7, 4,        // 左面(-X面)
        8, 9, 10, 10, 11, 8,    // 顶面(+Y面)
        12, 13, 14, 14, 15, 12,    // 底面(-Y面)
        16, 17, 18, 18, 19, 16, // 背面(+Z面)
        20, 21, 22, 22, 23, 20    // 正面(-Z面)
    };

    D3D11_SUBRESOURCE_DATA InitData;

    // 天空盒顶点缓冲区描述
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(SimpleVertex) * skyVertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    // 创建天空盒顶点缓冲区
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = &skyVertices[0];
    g_pd3dDevice->CreateBuffer(&vbd, &InitData, &g_pSkyVertexBuffer);
    // 天空盒索引缓冲区描述
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_DEFAULT;
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
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    //cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    g_pd3dDevice->CreateBuffer(&cbd, nullptr, &g_pSkyConstantBuffer);

    D3D11_DEPTH_STENCIL_DESC dsDesc;
    // 一般深度状态
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS; // less
    dsDesc.StencilEnable = false;
    g_pd3dDevice->CreateDepthStencilState(&dsDesc, &g_pDSSNodepthWrite);
    // 允许使用深度值一致的像素进行替换的深度/模板状态
    // 该状态用于绘制天空盒，因为深度值为1.0时默认无法通过深度测试
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // less equal
    dsDesc.StencilEnable = false;
    g_pd3dDevice->CreateDepthStencilState(&dsDesc, &g_pDSSLessEqual);


    // 立方体顶点缓冲区描述
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(SimpleVertex) * cubeVertexVec.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    // 创建立方体顶点缓冲区
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = &cubeVertexVec[0];
    g_pd3dDevice->CreateBuffer(&vbd, &InitData, &g_pCubeVertexBuffer);
    // 立方体索引缓冲区描述
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(WORD) * cubeIndexVec.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    // 创建立方体索引缓冲区
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = &cubeIndexVec[0];
    g_pd3dDevice->CreateBuffer(&ibd, &InitData, &g_pCubeIndexBuffer);
    // 常量缓冲区描述
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    g_pd3dDevice->CreateBuffer(&cbd, nullptr, &g_pCubeConstantBuffer);


    g_pTextureCubeSRVs.resize(6);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/sunset_posX.bmp", NULL, NULL, &g_pTextureCubeSRVs[0], NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/sunset_posY.bmp", NULL, NULL, &g_pTextureCubeSRVs[1], NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/sunset_posZ.bmp", NULL, NULL, &g_pTextureCubeSRVs[2], NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/sunset_negX.bmp", NULL, NULL, &g_pTextureCubeSRVs[3], NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/sunset_negY.bmp", NULL, NULL, &g_pTextureCubeSRVs[4], NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/sunset_negZ.bmp", NULL, NULL, &g_pTextureCubeSRVs[5], NULL);
    D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"models/texture/desertcube.dds", NULL, NULL, &g_pTextureSkySRV, NULL);
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
    }

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


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
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
    if (FAILED(hr))
        return hr;

    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pCubeSampler);

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

// sp 线起点 
// sq 线终点
// amin amax  表示 AABB包围盒 X, Y, Z 轴坐标的 最小 最大值
static float tmin, tmax;
static bool isectSegAABB(XMFLOAT4 &eye, XMFLOAT4 &target,
    XMFLOAT3 &min, XMFLOAT3 &max,
    float& tmin, float& tmax)
{
    static const float EPS = 1e-6f;
    float sp[3] = { eye.x, eye.y, eye.z };
    float sq[3] = { target.x, target.y, target.z };
    float amin[3] = { min.x, min.y, min.z };
    float amax[3] = { max.x, max.y, max.z };
    // 视线方向
    float d[3];
    d[0] = target.x - eye.x;
    d[1] = target.y - eye.y;
    d[2] = target.z - eye.z;
    // 因为是线段 所以参数t取值在0和1之间
    tmin = 0.0;
    tmax = 1.0f;

    for (int i = 0; i < 3; i++)
    {
        // 如果视线某一个轴分量为0，且在包围盒这个轴分量之外，那么直接判定不相交 
        if (fabsf(d[i]) < EPS)
        {
            if (sp[i] < amin[i] || sp[i] > amax[i])
                return false;
        }
        else
        {
            const float ood = 1.0f / d[i];
            // 计算参数t 并令 t1为较小值 t2为较大值
            float t1 = (amin[i] - sp[i]) * ood;
            float t2 = (amax[i] - sp[i]) * ood;
            if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }

            if (t1 > tmin) tmin = t1;
            if (t2 < tmax) tmax = t2;

            // 判定不相交
            if (tmin > tmax) return false;
        }
    }
    return true;
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
    if (mUsedCamera == 0) {
        Eye = XMLoadFloat4(&m_pFirstPersonCamera->GetPosition());
        At = XMLoadFloat4(&m_pFirstPersonCamera->GetTarget());
        g_View = XMMatrixLookAtLH(Eye, Eye + At, Up);
    }
    else {
        Eye = XMLoadFloat4(&m_pFreeLookCamera->GetPosition());
        At = XMLoadFloat4(&m_pFreeLookCamera->GetTarget());
        g_View = XMMatrixLookAtLH(Eye, Eye + At, Up);
    }

    XMFLOAT4 vCamera, vTarget;
    XMStoreFloat4(&vCamera, Eye);
    XMStoreFloat4(&vTarget, At);
    
    // 监听键盘
    DirectX::Keyboard::State keyState = m_pKeyboard->GetState();
    if (keyState.IsKeyDown(DirectX::Keyboard::D1))
    {
        mUsedCamera = 0;
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::D2))
    {
        mUsedCamera = 1;
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::D3))
    {
        mTexMode = 0;
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::D4))
    {
        mTexMode = 1;
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::W))
    {
        if (mUsedCamera == 0) {
            m_pFirstPersonCamera->MoveForwardBack(2.0f);
        }
        else {
            m_pFreeLookCamera->MoveForwardBack(2.0f);
        }
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::S))
    {
        if (mUsedCamera == 0) {
            m_pFirstPersonCamera->MoveForwardBack(-2.0f);
        }
        else {
            m_pFreeLookCamera->MoveForwardBack(-2.0f);
        }
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::A))
    {
        if (mUsedCamera == 0) {
            m_pFirstPersonCamera->MoveLeftRight(-0.5f);
        }
        else {
            m_pFreeLookCamera->MoveLeftRight(-0.5f);
        }
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::D))
    {
        if (mUsedCamera == 0) {
            m_pFirstPersonCamera->MoveLeftRight(0.5f);
        }
        else {
            m_pFreeLookCamera->MoveLeftRight(0.5f);
        }
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::Escape))
    {
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
    }
    if (keyState.IsKeyDown(DirectX::Keyboard::Space) && mUsedCamera == 0)
    {
        m_pFirstPersonCamera->Jump();
    }
    m_pFirstPersonCamera->CheckJump(0.2f, 10.0f, 20.0f);

    // 监听鼠标
    DirectX::Mouse::State mouseState = m_pMouse->GetState();
    DirectX::Mouse::ButtonStateTracker st;
    if (mouseState.positionMode == DirectX::Mouse::MODE_RELATIVE)
    {
        if (mUsedCamera == 0) {
            m_pFirstPersonCamera->Pitch(mouseState.y * 0.005f);
            m_pFirstPersonCamera->Yaw(mouseState.x * 0.005f);
        }
        else {
            m_pFreeLookCamera->Pitch(mouseState.y * 0.005f);
            m_pFreeLookCamera->Yaw(mouseState.x * 0.005f);
        }
        if (mouseState.leftButton) {
            if (isectSegAABB(vCamera, vTarget, treeAABB.MinPos, treeAABB.MaxPos, tmin, tmax)) {
                MessageBox(NULL, L"你选中了商店！", L"提示信息", MB_OK);
            }
        }
    }
    //
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.76f, 0.82f, 0.94f, 1.0f }; // red, green, blue, alpha
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
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
    };
    for (int i = 0; i < models.size(); i++) {
        XMMATRIX mWorld = modelWorlds[i];
        cb.mView = XMMatrixTranspose(g_View);
        cb.mProjection = XMMatrixTranspose(g_Projection);

        cb.vLightDir[0] = vLightDirs[0];
        cb.vLightDir[1] = vLightDirs[1];
        cb.vLightDir[2] = vLightDirs[2];
        cb.vLightColor[0] = vLightColors[0];
        cb.vLightColor[1] = vLightColors[1];
        cb.vLightColor[2] = vLightColors[2];
        cb.vCamera = vCamera;
        cb.vTarget = vTarget;

        AssimpModel::RenderModel(models[i]->mName, cb, mWorld);
    }
    /*ConstantBuffer tcb;
    tcb.mWorld = XMMatrixTranslation(0, 0, 0) * XMMatrixScaling(1, 0, 1);
    tcb.mView = XMMatrixTranspose(g_View);
    tcb.mProjection = XMMatrixTranspose(g_Projection);
    tcb.vLightDir[0] = vLightDirs[0];
    tcb.vLightDir[1] = vLightDirs[1];
    tcb.vLightDir[2] = vLightDirs[2];
    tcb.vLightColor[0] = vLightColors[0];
    tcb.vLightColor[1] = vLightColors[1];
    tcb.vLightColor[2] = vLightColors[2];
    m_pTerrain->Render(tcb, m_pTerrain->vertex_offset, m_pTerrain->index_offset);*/

    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    cb.mWorld = XMMatrixTranspose(XMMatrixScaling(5.0f, 5.0f, 5.0f) * XMMatrixTranslation(500.0f, 0.0f, 100.0f) * XMMatrixRotationY(0.0f));
    cb.mView = XMMatrixTranspose(g_View);
    cb.mProjection = XMMatrixTranspose(g_Projection);
    cb.vCamera = vCamera;
    cb.vTarget = vTarget;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pCubeVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pCubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    g_pImmediateContext->OMSetDepthStencilState(g_pDSSNodepthWrite, 0);
    g_pImmediateContext->UpdateSubresource(g_pCubeConstantBuffer, 0, NULL, &cb, 0, 0);
    g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCubeConstantBuffer);
    g_pImmediateContext->PSSetShader(g_pPixelShaders[0], NULL, 0);
    g_pImmediateContext->PSSetShaderResources(1, 1, &g_pTextureSkySRV);
    g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCubeConstantBuffer);
    g_pImmediateContext->PSSetSamplers(0, 1, &g_pCubeSampler);
    g_pImmediateContext->DrawIndexed(cubeIndexVec.size(), 0, 0);

    cb.mWorld = XMMatrixTranspose(XMMatrixIdentity());
    cb.mView = XMMatrixTranspose(g_View);
    cb.mProjection = XMMatrixTranspose(g_Projection);
    cb.vCamera = vCamera;
    cb.vTarget = vTarget;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pSkyVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pSkyIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    g_pImmediateContext->OMSetDepthStencilState(g_pDSSLessEqual, 0);
    g_pImmediateContext->UpdateSubresource(g_pSkyConstantBuffer, 0, NULL, &cb, 0, 0);
    g_pImmediateContext->VSSetShader(g_pSkyVertexShader, NULL, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pSkyConstantBuffer);
    g_pImmediateContext->PSSetShader(g_pPixelShaders[5], NULL, 0);
    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureSkySRV);
    g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pSkyConstantBuffer);
    g_pImmediateContext->PSSetSamplers(0, 1, &g_pCubeSampler);
    g_pImmediateContext->DrawIndexed(skyIndices.size(), 0, 0);
    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present(0, 0);
}
