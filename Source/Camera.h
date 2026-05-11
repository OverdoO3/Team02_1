#pragma once
#include "Component.h"
#include "DirectXMath.h"
#include "CameraBase.h"

class Actor;

class Camera : public Component, public CameraBase
{
public:
	COMPONENT_ID(Camera)
	Camera() {};
	~Camera() {};

	float fov = DirectX::XMConvertToRadians(60.0f);
	float aspect = 16.0f / 9.0f;
	float nearClip = 0.1f;
	float farClip = 1000.0f;

	bool mainCam;

	DirectX::XMMATRIX GetView() const override;
	DirectX::XMMATRIX GetProjection() const override;
	DirectX::XMFLOAT3 GetEye()const override;
	void LookAt(const DirectX::XMFLOAT3& target);
	void DrawInspector() override;

	void Serialize(json& j)const;
	void Deserialize(json& j);

	std::unique_ptr<Component> Clone() const override;
};