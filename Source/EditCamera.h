#pragma once
#include "DirectXMath.h"
#include <wtypes.h>
#include "CameraBase.h"
#include "Actor.h"

class EditorCamera : public CameraBase
{
public:
    void Update(float dt, bool isHovered,Actor* selectedActor);

    DirectX::XMMATRIX GetView() const;
    DirectX::XMMATRIX GetProjection() const;

    void SetAspect(float a) { aspect = a; }

    inline DirectX::XMFLOAT3 ToFloat3(DirectX::XMVECTOR v)
    {
        DirectX::XMFLOAT3 result;
        DirectX::XMStoreFloat3(&result, v);
        return result;
    }

    void LookAt(const DirectX::XMFLOAT3& target)
    {
        DirectX::XMFLOAT3 dir = {
            target.x - position.x,
            target.y - position.y,
            target.z - position.z
        };

        // 正規化
        float len = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        if (len > 0.0001f)
        {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }

        // pitch（上下）
        pitch = asinf(dir.y);

        // yaw（左右）
        yaw = atan2f(dir.x, dir.z);
    }

    DirectX::XMFLOAT3 GetEye()const override { return position; }


private:
    DirectX::XMFLOAT3 position = { 0.0f, 3.0f, 0.0f };
    float yaw = 0;
    float pitch = 0;

    DirectX::XMFLOAT3 focusTarget{};
    bool isFocus;

    float moveSpeed = 5.0f;
    float mouseSensitivity = 0.01f;

    float aspect = 16.0f / 9.0f;

    POINT prevMousePos{};
    bool rotating = false;
};