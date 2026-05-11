#pragma once
#include "Component.h"
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Transform : public Component
{
private:
	DirectX::XMFLOAT3 localPosition{ 0,0,0 };
	DirectX::XMFLOAT4 localRotation{ 0,0,0,1 };
	DirectX::XMFLOAT3 localScale{ 1,1,1 };

	DirectX::XMFLOAT3 worldPosition{ 0,0,0 };
	DirectX::XMFLOAT4 worldRotation{ 0,0,0,1 };
	DirectX::XMFLOAT3 worldScale{ 1,1,1 };

	DirectX::XMFLOAT3 euler{ 0,0,0 };

	DirectX::XMFLOAT4X4 localMatrix{};
	DirectX::XMFLOAT4X4 worldMatrix{};

	DirectX::XMFLOAT3 velocity{};

	Transform* parent = nullptr;
	std::vector<Transform*> children;
public:
	Transform() {};
	~Transform() {};
	
	void OnAwake(float elapsedTime)override;
	void UpdateTransform();

	void SetParent(Transform* newParent);
	std::unique_ptr<Component> Clone() const override;

	const DirectX::XMFLOAT4X4& GetWorldMatrix() const;

	DirectX::XMFLOAT3 GetForward() const;
	DirectX::XMFLOAT3 GetRight() const;
	DirectX::XMFLOAT3 GetUp() const;

	DirectX::XMFLOAT3 GetLocalPosition() const { return localPosition; }
	DirectX::XMFLOAT4 GetLocalRotation() const { return localRotation; }
	DirectX::XMFLOAT3 GetLocalScale() const { return localScale;}

	DirectX::XMFLOAT3 GetWorldPosition() const {
		return {
worldMatrix._41,
worldMatrix._42,
worldMatrix._43
		};
	}
	DirectX::XMFLOAT4 GetWorldRotation() const { return worldRotation; }
	DirectX::XMFLOAT3 GetWorldScale() const { return worldScale; }

	DirectX::XMFLOAT3 GetVelocity() const { return velocity; }

	void SetLocalPosition(const DirectX::XMFLOAT3& pos) { localPosition = pos; }
	void SetLocalRotation(const DirectX::XMFLOAT4& rot) { localRotation = rot; }
	void SetLocalScale(const DirectX::XMFLOAT3& scale) { this->localScale = scale; }

	void SetWorldPosition(const DirectX::XMFLOAT3& pos) 
	{
		if (parent)
		{
			parent->UpdateTransform();

			auto p = parent->GetWorldPosition();

			localPosition.x = pos.x - p.x;
			localPosition.y = pos.y - p.y;
			localPosition.z = pos.z - p.z;
		}
		else
		{
			localPosition = pos;
		}
	}
	void SetWorldRotation(const DirectX::XMFLOAT4& rot) { worldRotation = rot; }
	void SetWorldScale(const DirectX::XMFLOAT3& scale) { this->worldScale = scale; }

	void SetVelocity(const DirectX::XMFLOAT3& velocity) { this->velocity = velocity; }

	void SetRotationEuler(float pitch, float yaw, float roll);
	void SetRotationEulerPitch(float pitch);
	void SetRotationEulerYaw(float yaw);
	void SetRotationEulerRoll(float roll);
	void RotateAxis(DirectX::XMFLOAT3 axis, float angle);

	void LookAt(const DirectX::XMFLOAT3& target);
	void RotateEuler(float pitch, float yaw, float roll);

	DirectX::XMFLOAT3 GetEulerRotation() const;

	void DrawInspector()
	{
		float rot[3] =
		{
			DirectX::XMConvertToDegrees(euler.x),
			DirectX::XMConvertToDegrees(euler.y),
			DirectX::XMConvertToDegrees(euler.z)
		};

		ImGui::DragFloat3("Position", &localPosition.x, 0.001f);

		if (ImGui::DragFloat3("Rotation", rot, 0.001f))
		{
			SetRotationEuler(
				DirectX::XMConvertToRadians(rot[0]),
				DirectX::XMConvertToRadians(rot[1]),
				DirectX::XMConvertToRadians(rot[2])
			);
		}

		ImGui::DragFloat3("Scale", &localScale.x, 0.001f);
	}

	void Serialize(nlohmann::json& j)const override;
	void Deserialize(nlohmann::json& j)override;

	COMPONENT_ID(Transform)
};