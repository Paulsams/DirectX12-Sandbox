﻿#include <CubeRenderer.h>

#include <Application.h>
#include <CommandQueue.h>
#include <direct.h>
#include <filesystem>
#include <Helpers.h>
#include <iostream>
#include <Window.h>
#include <Mathf.h>

#include <wrl.h>
using namespace Microsoft::WRL;

#include <d3dx12.h>
#include <d3dcompiler.h>

#include <algorithm> // For std::min and std::max.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

using namespace DirectX;

// Vertex data for a colored cube.
struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

struct PipelineStateStream
{
    CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
    CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
    CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
    CD3DX12_PIPELINE_STATE_STREAM_VS VS;
    CD3DX12_PIPELINE_STATE_STREAM_PS PS;
    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
    CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
} pipelineStateStream;

static VertexPosColor g_Vertices[8] = {
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
    { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
    { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
    { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
    { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
};

static WORD g_Indexes[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

CubeRenderer::CubeRenderer(const std::wstring& name, int width, int height, bool vSync)
    : base( name, width, height, vSync )
      , m_Viewport( CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height) ) )
      , m_ScissorRect( CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ) )
      , m_camera(45.0, c_DirectNearZ, c_DirectFarZ)
      , m_directionMoveCamera( XMVectorZero() )
    {
        m_camera.Position = XMVectorSet(0, 0, 10, 1);
    }

void CubeRenderer::UpdateBufferResource(
    ComPtr<ID3D12GraphicsCommandList2> commandList,
    ID3D12Resource** pDestinationResource, 
    ID3D12Resource** pIntermediateResource,
    size_t numElements, size_t elementSize, const void* bufferData, 
    D3D12_RESOURCE_FLAGS flags)
{
    auto device = Application::Get().GetDevice();

    size_t bufferSize = numElements * elementSize;
    // Create a committed resource for the GPU resource in a default heap.
    {
        const auto heapProps    = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);
        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(pDestinationResource)));
    }

    // Create an committed resource for the upload.
    if (bufferData)
    {
        const auto heapProps    = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(pIntermediateResource)));

        D3D12_SUBRESOURCE_DATA subresourceData;
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = static_cast<LONG_PTR>(bufferSize);
        subresourceData.SlicePitch = subresourceData.RowPitch;

        ::UpdateSubresources(commandList.Get(), 
            *pDestinationResource, *pIntermediateResource,
            0, 0, 1, &subresourceData);
    }
}

void CubeRenderer::CreatePipelineState(ComPtr<ID3DBlob> vertexShaderBlob, ComPtr<ID3DBlob> pixelShaderBlob) {
    const auto device = Application::Get().GetDevice();

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
          D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
          D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData;
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0,
                                      D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters,
                                      0, nullptr, rootSignatureFlags);

    // Serialize the root signature.
    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
                                                        featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
    // Create the root signature.
    ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), 
                                                            rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets      = 1;
    rtvFormats.RTFormats[0]          = DXGI_FORMAT_R8G8B8A8_UNORM;

    pipelineStateStream.pRootSignature        = m_RootSignature.Get();
    pipelineStateStream.InputLayout           = { inputLayout, _countof(inputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.DSVFormat             = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats            = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));
}


std::optional<std::tuple<ComPtr<ID3DBlob>, ComPtr<ID3DBlob>>> CubeRenderer::LoadShaders()
{
    #if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    #else
    UINT compileFlags = 0;
    #endif

    static char pBuf[1024];
    _getcwd(pBuf, 1024);
    std::filesystem::path path = pBuf;
    path = path.parent_path().parent_path();
    auto vertexPath = path / L"Sandbox/shaders/VertexShader.hlsl";
    auto pixelPath = path / L"Sandbox/shaders/PixelShader.hlsl";

    ComPtr<ID3DBlob> vertexShaderBlob;
    ComPtr<ID3DBlob> pixelShaderBlob;
    ID3DBlob* errors;
    try
    {
        ThrowIfFailed(D3DCompileFromFile(vertexPath.c_str(), nullptr, nullptr,
        "main", "vs_5_1", compileFlags, 0, &vertexShaderBlob, &errors));

        ThrowIfFailed(D3DCompileFromFile(pixelPath.c_str(), nullptr, nullptr,
        "main", "ps_5_1", compileFlags, 0, &pixelShaderBlob, &errors));
    }
    catch (const std::exception&)
    {
        const char* errStr = static_cast<const char*>(errors->GetBufferPointer());
        std::cout << errStr;
        return {};
    }

    return { std::make_tuple(vertexShaderBlob, pixelShaderBlob) };
}

bool CubeRenderer::LoadContent()
{
    const auto device= Application::Get().GetDevice();
    const auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    const auto commandList  = commandQueue->GetCommandList();

    // Upload vertex buffer data.
    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    UpdateBufferResource(commandList.Get(),
        &m_VertexBuffer, &intermediateVertexBuffer,
        _countof(g_Vertices), sizeof(VertexPosColor), g_Vertices);

    // Create the vertex buffer view.
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = sizeof(g_Vertices);
    m_VertexBufferView.StrideInBytes = sizeof(VertexPosColor);

    // Upload index buffer data.
    ComPtr<ID3D12Resource> intermediateIndexBuffer;
    UpdateBufferResource(commandList.Get(),
        &m_IndexBuffer, &intermediateIndexBuffer,
        _countof(g_Indexes), sizeof(WORD), g_Indexes);

    // Create index buffer view.
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_IndexBufferView.SizeInBytes = sizeof(g_Indexes);

    // Create the descriptor heap for the depth-stencil view.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

    auto maybeShadersBlob = LoadShaders();
    if (!maybeShadersBlob.has_value())
        return false;
    auto [vertexShaderBlob, pixelShaderBlob] = maybeShadersBlob.value();

    CreatePipelineState(vertexShaderBlob, pixelShaderBlob);

    auto fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    m_ContentLoaded = true;

    // Resize/Create the depth buffer.
    ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    return true;
}

void CubeRenderer::ResizeDepthBuffer(int width, int height)
{
    if (m_ContentLoaded)
    {
        // Flush any GPU commands that might be referencing the depth buffer.
        Application::Get().Flush();

        width = std::max(1, width);
        height = std::max(1, height);

        auto device = Application::Get().GetDevice();

        // Resize screen dependent resources.
        // Create a depth buffer.
        D3D12_CLEAR_VALUE optimizedClearValue;
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = { 1.0f, 0 };

        const auto heapProps    = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
                1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS(&m_DepthBuffer)
        ));
        
        // Update the depth-stencil view.
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsvDesc,
            m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
    }
}

void CubeRenderer::OnResize(ResizeEventArgs& resizeArgs)
{
    if (resizeArgs.Width != GetClientWidth() || resizeArgs.Height != GetClientHeight())
    {
        base::OnResize(resizeArgs);

        m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
            static_cast<float>(resizeArgs.Width), static_cast<float>(resizeArgs.Height));

        ResizeDepthBuffer(resizeArgs.Width, resizeArgs.Height);
    }
}

void CubeRenderer::UnloadContent()
{
    m_ContentLoaded = false;
}

void CubeRenderer::MoveCamera(XMVECTOR direction, double deltaTime) {
    constexpr float moveSpeed = 10.0f;

    auto offsetMatrix = XMMatrixTranslationFromVector(direction * moveSpeed * deltaTime);
    auto rotationMatrix = XMMatrixTranspose(XMMatrixRotationQuaternion(m_camera.Rotation));
    auto matrix = XMMatrixTranslationFromVector(m_camera.Position) -
        XMMatrixMultiply(offsetMatrix, rotationMatrix);
    m_camera.Position = XMVectorSetW(matrix.r[3], 1.0f);
}

void CubeRenderer::OnUpdate(UpdateEventArgs& updateArgs)
{
    base::OnUpdate(updateArgs);
    
    static uint32_t frameCount = 0;
    static double totalTime = 0.0;

    totalTime += updateArgs.ElapsedTime;
    frameCount++;

    if (totalTime > 1.0)
    {
        double fps = frameCount / totalTime;

        char buffer[512];
        sprintf_s(buffer, "FPS: %f\n", fps);
        //OutputDebugStringA(buffer);
        std::cout << buffer;

        frameCount = 0;
        totalTime = 0.0;

        if (!XMVector3Equal(m_directionMoveCamera, XMVectorZero()))
        {
            auto position = m_camera.Position.m128_f32;
            std::cout << "Changed camera position: " << std::printf("X: %f; Y: %f; Z: %f", position[0],
                                                                    position[1], position[2]) << std::endl;
        }
    }

    // Update the model matrix.
    const float angle = static_cast<float>(updateArgs.TotalTime) * 90.0f;
    const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
    m_ModelMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

    if (!XMVector3Equal(m_directionMoveCamera, XMVectorZero()))
        MoveCamera(m_directionMoveCamera, updateArgs.ElapsedTime);
}

// Transition a resource
void CubeRenderer::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
    ComPtr<ID3D12Resource> resource,
    D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(),
        beforeState, afterState);

    commandList->ResourceBarrier(1, &barrier);
}

// Clear a render target.
void CubeRenderer::ClearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList,
    D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void CubeRenderer::ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList,
    D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth)
{
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

XMMATRIX CubeRenderer::getProjectionMatrix(const Camera& camera, float totalTime)
{
    constexpr float shakenFunctionTime = 30.0f;
    constexpr float shakenPixels = 50.0f;

    const float aspectRatio = GetClientWidth() / static_cast<float>(GetClientHeight());
    if (!m_isShaken)
        return camera.getProjectionMatrix(aspectRatio);

    float coefficientX = (math::PingPong(totalTime, 1.0f / shakenFunctionTime) - 0.5f / shakenFunctionTime) * 2.0f;
    float coefficientY = (math::PingPong(totalTime + 1.0f, 1.17f / shakenFunctionTime) - (1.17f / 2.0f) / shakenFunctionTime) * 2.0f;
    float shiftX = coefficientX * shakenFunctionTime * shakenPixels / GetClientWidth();
    float shiftY = coefficientY * shakenFunctionTime * shakenPixels / GetClientHeight();
    return camera.getProjectionMatrixWithJitter(aspectRatio, shiftX, shiftY);
}

void CubeRenderer::OnRender(RenderEventArgs& renderArgs)
{
    base::OnRender(renderArgs);

    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
    auto backBuffer = m_pWindow->GetCurrentBackBuffer();
    auto rtv = m_pWindow->GetCurrentRenderTargetView();
    auto dsv = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

    // Clear the render targets.
    {
        TransitionResource(commandList, backBuffer,
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

        ClearRTV(commandList, rtv, clearColor);
        ClearDepth(commandList, dsv);
    }
    
    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    commandList->SetPipelineState(m_PipelineState.Get());
    commandList->SetGraphicsRootSignature(m_RootSignature.Get());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    commandList->IASetIndexBuffer(&m_IndexBufferView);

    commandList->OMSetRenderTargets(1, &rtv,
        FALSE, &dsv);

    // Update the MVP matrix
    XMMATRIX mvpMatrix = XMMatrixMultiply(m_ModelMatrix, m_camera.getViewMatrix());
    mvpMatrix = XMMatrixMultiply(mvpMatrix, getProjectionMatrix(m_camera, renderArgs.TotalTime));
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

    commandList->DrawIndexedInstanced(_countof(g_Indexes), 1, 0, 0, 0);

    // Present
    {
        TransitionResource(commandList, backBuffer,
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
        
        currentBackBufferIndex = m_pWindow->Present();
        
        commandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);
    }
}

void CubeRenderer::OnKeyPressed(KeyEventArgs& keyArgs)
{
    base::OnKeyPressed(keyArgs);

    m_directionMoveCamera = XMVectorClamp(m_directionMoveCamera + getMoveDirectionFromInput(keyArgs.Key),
        XMVectorSplatOne() * -1.0f, XMVectorSplatOne() * 1.0f);
    
    switch (keyArgs.Key)
    {
        case KeyCode::Escape:
            Application::Get().Quit(0);
            break;
        case KeyCode::Enter:
            if (keyArgs.Alt)
            {
                case KeyCode::F11:
                    m_pWindow->ToggleFullscreen();
                break;
            }
        case KeyCode::V:
            m_pWindow->ToggleVSync();
            break;
        case KeyCode::Z:
            m_IsFlippedZ = !m_IsFlippedZ;
            if (m_IsFlippedZ)
                m_camera.setPlanes(c_DirectFarZ, c_DirectNearZ);
            else
                m_camera.setPlanes(c_DirectNearZ, c_DirectFarZ);
            std::cout << "IsFlippedZ: " << (m_IsFlippedZ ? "ON" : "OFF") << std::endl;
            break;
        case KeyCode::H:
            m_isShaken = !m_isShaken;
            std::cout << "Jitter: " << (m_isShaken ? "ON" : "OFF") << std::endl;
            break;
        case KeyCode::R:
            auto maybeShadersBlob = LoadShaders();
            if (!maybeShadersBlob.has_value())
                break;
            auto [vertexShaderBlob, pixelShaderBlob] = maybeShadersBlob.value();
        
            CreatePipelineState(vertexShaderBlob, pixelShaderBlob);
            std::cout << "Recompile Shaders" << std::endl;
            break;
    }
}

void CubeRenderer::OnKeyReleased(KeyEventArgs& keyArgs)
{
    Game::OnKeyReleased(keyArgs);

    m_directionMoveCamera = XMVectorClamp(m_directionMoveCamera - getMoveDirectionFromInput(keyArgs.Key),
        XMVectorSplatOne() * -1.0f, XMVectorSplatOne() * 1.0f);
}

XMVECTOR CubeRenderer::getMoveDirectionFromInput(KeyCode::Key keyCode)
{
    switch (keyCode)
    {
        case KeyCode::Q:
            return XMVectorSetY(XMVectorZero(), -1.0f);
        case KeyCode::E:
            return XMVectorSetY(XMVectorZero(), 1.0f);
        case KeyCode::A:
            return XMVectorSetX(XMVectorZero(), -1.0f);
        case KeyCode::D:
            return XMVectorSetX(XMVectorZero(), 1.0f);
        case KeyCode::S:
            return XMVectorSetZ(XMVectorZero(), -1.0f);
        case KeyCode::W:
            return XMVectorSetZ(XMVectorZero(), 1.0f);
    }

    return XMVectorZero();
}

void CubeRenderer::OnMouseMoved(MouseMotionEventArgs& mouseMotionArgs)
{
    base::OnMouseMoved(mouseMotionArgs);
    
    if (!mouseMotionArgs.LeftButton)
        return;
    
    m_camera.Rotation = XMQuaternionMultiply(m_camera.Rotation, XMQuaternionRotationRollPitchYaw(
        static_cast<float>(mouseMotionArgs.RelY) / GetClientHeight(),static_cast<float>(mouseMotionArgs.RelX) / GetClientWidth(), 0.0f));
}

void CubeRenderer::OnMouseWheel(MouseWheelEventArgs& mouseWheelArgs)
{
    base::OnMouseWheel(mouseWheelArgs);
    
    m_camera.FoV = std::clamp(m_camera.FoV - mouseWheelArgs.WheelDelta, 12.0f, 90.0f);
    std::cout << "FoV: " << m_camera.FoV << std::endl;
}
