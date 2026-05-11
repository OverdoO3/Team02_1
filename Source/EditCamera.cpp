#include "EditCamera.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "algorithm"
#include "DirectXCommon.h"
#include "Lerp.h"
#include "Input.h"

void EditorCamera::Update(float elapsedTime, bool isHovered,Actor* selectedActor)
{
    if (!isHovered) return;

    auto& io = ImGui::GetIO();

    // Ž‹“_‰ñ“]
    if (rotating)
    {
        POINT current;
        GetCursorPos(&current);

        float dx = float(current.x - prevMousePos.x);
        float dy = float(current.y - prevMousePos.y);

        yaw -= dx * mouseSensitivity;
        pitch += dy * mouseSensitivity;

        // ã‰º§ŒÀiŽñÜ‚ê–hŽ~j
        pitch = std::clamp(pitch, -1.4f, 1.4f);

        prevMousePos = current;
    }

    // ˆÚ“®•ûŒüƒxƒNƒgƒ‹ŒvŽZ
    DirectX::XMVECTOR forward =
        DirectX::XMVectorSet(
            cosf(pitch) * sinf(yaw),
            sinf(pitch),
            cosf(pitch) * cosf(yaw),
            0.0f);

    DirectX::XMVECTOR right =
        DirectX::XMVector3Cross(
            DirectX::XMVectorSet(0, 1, 0, 0),
            forward);

    DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);

    float speed = moveSpeed;
    if (ImGui::GetIO().KeyShift)
        speed *= 3.0f;

    using namespace DirectX;

    if (isFocus)
    {
        DirectX::XMFLOAT3 currentPos = position;

        // –Ú•WˆÊ’u
        DirectX::XMFLOAT3 dir = currentPos - focusTarget;
        float len = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);

        if (len > 0.0001f)
        {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }

        float desiredDistance = 10.0f;

        DirectX::XMFLOAT3 targetPos = {
            focusTarget.x + dir.x * desiredDistance,
            focusTarget.y + dir.y * desiredDistance,
            focusTarget.z + dir.z * desiredDistance
        };

        currentPos = Lerp(currentPos, targetPos, 0.1f);

        position = currentPos;
        LookAt(targetPos);

        // ‹ß‚Ã‚¢‚½‚çŽ~‚ß‚é
        float dist = sqrtf(
            (currentPos.x - targetPos.x) * (currentPos.x - targetPos.x) +
            (currentPos.y - targetPos.y) * (currentPos.y - targetPos.y) +
            (currentPos.z - targetPos.z) * (currentPos.z - targetPos.z)
        );

        if (dist < 0.01f)
            isFocus = false;
    }
    else
    {

        if (InputC::MouseDown(VK_MBUTTON))
        {
            ImVec2 delta = ImGui::GetIO().MouseDelta;

            position = position + ToFloat3((-right * delta.x + up * delta.y) * 0.1f);
        }

        float wheel = ImGui::GetIO().MouseWheel;

        if (wheel != 0.0f)
        {
            position = position + ToFloat3(forward * wheel * 2.0f);
        }

        if (InputC::KeyDown(VK_MENU) && InputC::KeyDown(VK_LBUTTON))
        {
            ImVec2 delta = ImGui::GetIO().MouseDelta;

            yaw += delta.x * mouseSensitivity;
            pitch += delta.y * mouseSensitivity;
        }

        if (InputC::KeyPressed('F') && selectedActor)
        {
            focusTarget = selectedActor->GetComponent<Transform>()->GetWorldPosition();
            isFocus = true;
        }

    }
}

DirectX::XMMATRIX EditorCamera::GetView() const
{
    using namespace DirectX;

    XMVECTOR pos = XMLoadFloat3(&position);

    XMVECTOR forward =
        XMVectorSet(
            cosf(pitch) * sinf(yaw),
            sinf(pitch),
            cosf(pitch) * cosf(yaw),
            0.0f);

    return XMMatrixLookToLH(pos, forward, XMVectorSet(0, 1, 0, 0));
}

DirectX::XMMATRIX EditorCamera::GetProjection() const
{
    return DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(60.0f),
        aspect,
        0.1f,
        1000.0f);
}