#include "Transform.h"
#include "Factory.h"

REGISTER_COMPONENT(ComponentID::Transform, Transform)

void Transform::OnAwake(float elapsedTime)
{
    UpdateTransform();
}

void Transform::UpdateTransform()
{
	using namespace DirectX;

	XMMATRIX S = XMMatrixScaling(localScale.x, localScale.y, localScale.z);

    XMVECTOR q = XMLoadFloat4(&localRotation);
    XMMATRIX R = XMMatrixRotationQuaternion(q);

	XMMATRIX T = XMMatrixTranslation(localPosition.x,localPosition.y, localPosition.z);

	XMMATRIX local = S * R * T;

	XMStoreFloat4x4(&localMatrix, local);

    //親ありバージョン
    if (parent)
    {
        XMMATRIX parentWorld = XMLoadFloat4x4(&parent->worldMatrix);
        XMMATRIX world = local * parentWorld;

        XMStoreFloat4x4(&worldMatrix, world);
    }
    else
    {
        worldMatrix = localMatrix;
    }

    for (auto child : children)
    {
        child->UpdateTransform();
    }
}

void Transform::SetParent(Transform* newParent)
{
    if (parent)
    {
        auto& siblings = parent->children;
        siblings.erase(
            std::remove(siblings.begin(), siblings.end(), this),
            siblings.end());
    }

    parent = newParent;

    if (parent)
        parent->children.push_back(this);
}

std::unique_ptr<Component> Transform::Clone() const
{
    auto c = std::make_unique<Transform>();

    c->SetLocalPosition(localPosition);
    c->SetLocalRotation(localRotation);
    c->SetLocalScale(localScale);

    return c;
}

DirectX::XMFLOAT3 Transform::GetForward() const
{
    DirectX::XMVECTOR q = XMLoadFloat4(&localRotation);

    DirectX::XMVECTOR forward = DirectX::XMVector3Rotate(
        DirectX::XMVectorSet(0, 0, 1, 0),
        q);

    DirectX::XMFLOAT3 result;
    XMStoreFloat3(&result, forward);

    return result;
}

DirectX::XMFLOAT3 Transform::GetRight() const
{
    DirectX::XMVECTOR q = XMLoadFloat4(&localRotation);

    DirectX::XMVECTOR forward = DirectX::XMVector3Rotate(
        DirectX::XMVectorSet(1, 0, 0, 0),
        q);

    DirectX::XMFLOAT3 result;
    XMStoreFloat3(&result, forward);

    return result;
}

DirectX::XMFLOAT3 Transform::GetUp() const
{
    DirectX::XMVECTOR q = XMLoadFloat4(&localRotation);

    DirectX::XMVECTOR forward = DirectX::XMVector3Rotate(
        DirectX::XMVectorSet(0, 1, 0, 0),
        q);

    DirectX::XMFLOAT3 result;
    XMStoreFloat3(&result, forward);

    return result;
}

const DirectX::XMFLOAT4X4& Transform::GetWorldMatrix() const
{
    return worldMatrix;
}

void Transform::SetRotationEuler(float pitch, float yaw, float roll)
{
    euler = { pitch,yaw,roll };

    DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
    q = DirectX::XMQuaternionNormalize(q);

    DirectX::XMStoreFloat4(&localRotation, q);
}

void Transform::SetRotationEulerPitch(float pitch)
{
    euler = { pitch,euler.y,euler.z };

    DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
    q = DirectX::XMQuaternionNormalize(q);

    DirectX::XMStoreFloat4(&localRotation, q);
}

void Transform::SetRotationEulerYaw(float yaw)
{
    euler = { euler.x,yaw,euler.z };

    DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
    q = DirectX::XMQuaternionNormalize(q);

    DirectX::XMStoreFloat4(&localRotation, q);
}

void Transform::SetRotationEulerRoll(float roll)
{
    euler = { euler.x ,euler.y,roll };

    DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
    q = DirectX::XMQuaternionNormalize(q);

    DirectX::XMStoreFloat4(&localRotation, q);
}

void Transform::RotateAxis(DirectX::XMFLOAT3 axis, float angle)
{
    DirectX::XMVECTOR q1 = XMLoadFloat4(&localRotation);
    DirectX::XMVECTOR q2 = DirectX::XMQuaternionRotationAxis(XMLoadFloat3(&axis), angle);

    q1 = DirectX::XMQuaternionMultiply(q1, q2);
    q1 = DirectX::XMQuaternionNormalize(q1);

    DirectX::XMStoreFloat4(&localRotation, q1);
}

void Transform::LookAt(const DirectX::XMFLOAT3& target)
{
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&localPosition);
    DirectX::XMVECTOR tar = XMLoadFloat3(&target);

    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, tar, DirectX::XMVectorSet(0, 1, 0, 0));

    DirectX::XMMATRIX inv = DirectX::XMMatrixInverse(nullptr, view);

    DirectX::XMVECTOR q = DirectX::XMQuaternionRotationMatrix(inv);

    DirectX::XMStoreFloat4(&localRotation, q);
}

void Transform::RotateEuler(float pitch, float yaw, float roll)
{
    DirectX::XMVECTOR q1 = DirectX::XMLoadFloat4(&localRotation);
    DirectX::XMVECTOR q2 = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

    q1 = DirectX::XMQuaternionMultiply(q1, q2);
    q1 = DirectX::XMQuaternionNormalize(q1);

    DirectX::XMStoreFloat4(&localRotation, q1);
}

DirectX::XMFLOAT3 Transform::GetEulerRotation() const
{
    return euler;
}

void Transform::Serialize(nlohmann::json& j) const
{
    auto safe = [](float v)
        {
            return std::isfinite(v) ? v : 0.0f;
        };

    j["position"] = { safe(localPosition.x), safe(localPosition.y), safe(localPosition.z) };
    j["rotation"] = { safe(localRotation.x), safe(localRotation.y), safe(localRotation.z),safe(localRotation.w) };
    j["scale"] = { safe(localScale.x), safe(localScale.y), safe(localScale.z)};
}

void Transform::Deserialize(nlohmann::json& j)
{
    auto p = j["position"];
    localPosition = { p[0], p[1], p[2] };

    auto r = j["rotation"];
    localRotation = { r[0], r[1], r[2], r[3]};

    auto s = j["scale"];
    localScale = { s[0], s[1], s[2] };

    UpdateTransform();
}