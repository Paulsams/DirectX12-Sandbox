#include "Camera.h"

using namespace DirectX;

Camera::Camera( float fov, float nearZ, float farZ )
    : FoV( fov )
    , m_NearZ( nearZ )
    , m_FarZ( farZ )
    , Position( XMVectorZero() )
    , Rotation( g_XMIdentityR3 ) { }

void Camera::setPlanes(float nearZ, float farZ)
{
    m_NearZ = nearZ;
    m_FarZ  = farZ;
}

XMMATRIX Camera::getProjectionMatrix(float aspectRatio) const
{
    return XMMatrixPerspectiveFovLH(XMConvertToRadians(FoV), aspectRatio, m_NearZ, m_FarZ);
}

XMMATRIX Camera::getProjectionMatrixWithJitter(float aspectRatio, float shiftX, float shiftY) const
{
    auto projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(FoV), aspectRatio, m_NearZ, m_FarZ);
    projectionMatrix.r[2].m128_f32[0] = shiftX;
    projectionMatrix.r[2].m128_f32[1] = shiftY;

    return projectionMatrix;
}

XMMATRIX Camera::getViewMatrix() const
{
    return XMMatrixMultiply(XMMatrixTranslationFromVector(Position), XMMatrixRotationQuaternion(Rotation));
}




