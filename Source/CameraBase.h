#pragma once
#include "DirectXMath.h"

class CameraBase
{
public:
    virtual DirectX::XMMATRIX GetView() const = 0;
    virtual DirectX::XMMATRIX GetProjection() const = 0;
    virtual DirectX::XMFLOAT3 GetEye() const = 0;
};