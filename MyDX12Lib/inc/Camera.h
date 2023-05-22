#pragma once
#include <DirectXMath.h>

class Camera
{
public:
    Camera(float fov, float nearZ, float farZ);
    
    void setPlanes(float nearZ, float farZ);

    DirectX::XMMATRIX getProjectionMatrix(float aspectRatio) const;
    DirectX::XMMATRIX getProjectionMatrixWithJitter(float aspectRatio, float shiftX, float shiftY) const;

    DirectX::XMMATRIX getViewMatrix() const;

    DirectX::XMVECTOR Position;
    DirectX::XMVECTOR Rotation;
    float FoV;
private:
    float m_NearZ;
    float m_FarZ;
};
