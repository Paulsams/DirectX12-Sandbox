#pragma once

#include <DirectXMath.h>
#include <intsafe.h>
#include <vector>

class Mesh
{
public:
    Mesh(std::vector<DirectX::XMFLOAT3>&& vertices,
         std::vector<WORD>&& indexes);

    Mesh(const std::vector<DirectX::XMFLOAT3>& vertices,
         const std::vector<WORD>& indexes);

private:
    std::vector<DirectX::XMFLOAT3> vertices;
    std::vector<WORD> indexes;
};

namespace primitives
{
    Mesh* createCube();
}
