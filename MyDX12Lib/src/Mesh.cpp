#include "Mesh.h"

using namespace DirectX;

Mesh::Mesh( std::vector<XMFLOAT3>&& vertices, std::vector<WORD>&& indexes ):
    vertices(std::move(vertices)),
    indexes(std::move(indexes)) { }

Mesh::Mesh( const std::vector<XMFLOAT3>& vertices, const std::vector<WORD>& indexes ):
    vertices(vertices),
    indexes(indexes) { }

namespace primitives
{
    Mesh* createCube()
    {
        static auto mesh = new Mesh({
            { XMFLOAT3(-1.0f, -1.0f, -1.0f) }, // 0
            { XMFLOAT3(-1.0f,  1.0f, -1.0f) }, // 1
            { XMFLOAT3( 1.0f,  1.0f, -1.0f) }, // 2
            { XMFLOAT3( 1.0f, -1.0f, -1.0f) }, // 3
            { XMFLOAT3(-1.0f, -1.0f,  1.0f) }, // 4
            { XMFLOAT3(-1.0f,  1.0f,  1.0f) }, // 5
            { XMFLOAT3( 1.0f,  1.0f,  1.0f) }, // 6
            { XMFLOAT3( 1.0f, -1.0f,  1.0f) }  // 7
        }, {
            0, 1, 2, 0, 2, 3,
            4, 6, 5, 4, 7, 6,
            4, 5, 1, 4, 1, 0,
            3, 2, 6, 3, 6, 7,
            1, 5, 6, 1, 6, 2,
            4, 0, 3, 4, 3, 7
        });

        return mesh;
    }
}
