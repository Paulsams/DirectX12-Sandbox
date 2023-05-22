#pragma once

#include "Camera.h"
#include <Game.h>
#include <Window.h>

#include <DirectXMath.h>

class CubeRenderer : public Game
{
public:
    CubeRenderer(const std::wstring& name, int width, int height, bool vSync = false);
    /**
     *  Load content required for the demo.
     */
    bool LoadContent() override;

    /**
     *  Unload demo specific content that was loaded in LoadContent.
     */
    void UnloadContent() override;

protected:
    /**
     *  Update the game logic.
     */
    void OnUpdate(UpdateEventArgs& updateArgs) override;

    /**
     *  Render stuff.
     */
    void OnRender(RenderEventArgs& renderArgs) override;

    /**
     * Invoked by the registered window when a key is pressed
     * while the window has focus.
     */
    void OnKeyPressed(KeyEventArgs& keyArgs) override;

    void OnKeyReleased(KeyEventArgs& keyArgs) override;

    void OnMouseMoved(MouseMotionEventArgs& mouseMotionArgs) override;

    /**
     * Invoked when the mouse wheel is scrolled while the registered window has focus.
     */
    void OnMouseWheel(MouseWheelEventArgs& mouseWheelArgs) override;


    void OnResize(ResizeEventArgs& resizeArgs) override;

private:
    using base = Game;

    DirectX::XMVECTOR getMoveDirectionFromInput(KeyCode::Key keyCode);
    void MoveCamera(DirectX::XMVECTOR direction, double deltaTime);
    
    // Helper functions
    // Transition a resource
    void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
                            Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                            D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

    // Clear a render target view.
    void ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
                  D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

    // Clear the depth of a depth-stencil view.
    void ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
        D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

    DirectX::XMMATRIX getProjectionMatrix( const Camera& camera, float totalTime);

    // Create a GPU buffer.
    void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
        ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
        size_t numElements, size_t elementSize, const void* bufferData, 
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
    
    void CreatePipelineState(Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob,
                             Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob);

    // Resize the depth buffer to match the size of the client area.
    void ResizeDepthBuffer(int width, int height);

    std::optional<std::tuple<Microsoft::WRL::ComPtr<ID3DBlob>,
        Microsoft::WRL::ComPtr<ID3DBlob>>> LoadShaders();

    uint64_t m_FenceValues[Window::BufferCount] = {};
    
    // Vertex buffer for the cube.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    // Index buffer for the cube.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

    // Depth buffer.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthBuffer;
    // Descriptor heap for depth buffer.
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

    // Root signature
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

    // Pipeline state object.
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;

    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;

    const float c_DirectNearZ = 0.1f;
    const float c_DirectFarZ = 100.0f;
    bool m_IsFlippedZ = false;
    
    bool m_isShaken = false;

    DirectX::XMMATRIX m_ModelMatrix;
    Camera m_camera;
    DirectX::XMVECTOR m_directionMoveCamera;

    bool m_ContentLoaded;
};
