#include "Camera.h"
#include "Actor.h"
#include "Factory.h"
REGISTER_COMPONENT(ComponentID::Camera, Camera)

DirectX::XMMATRIX Camera::GetView() const
{
	using namespace DirectX;

	auto tran = owner->GetComponent<Transform>();

	DirectX::XMFLOAT3 pos = tran->GetWorldPosition();
	DirectX::XMFLOAT3 forward = tran->GetForward();
	DirectX::XMFLOAT3 up = tran->GetUp();

	DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(&pos);
	DirectX::XMVECTOR focus = eye + DirectX::XMLoadFloat3(&forward);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
		eye,
		focus,
		XMLoadFloat3(&up)
	);

	return view;
}

DirectX::XMMATRIX Camera::GetProjection() const
{
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(
		fov,
		aspect,
		nearClip,
		farClip
	);

	return proj;
}

DirectX::XMFLOAT3 Camera::GetEye() const
{
	return owner->GetComponent<Transform>()->GetWorldPosition();
}

void Camera::LookAt(const DirectX::XMFLOAT3& target)
{
	auto tran = owner->GetComponent<Transform>();

	DirectX::XMFLOAT3 pos = tran->GetWorldPosition();

	DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(&pos);
	DirectX::XMVECTOR focus = DirectX::XMLoadFloat3(&target);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eye, focus, up);

	DirectX::XMMATRIX inv = DirectX::XMMatrixInverse(nullptr, view);

	DirectX::XMVECTOR rot = DirectX::XMQuaternionRotationMatrix(inv);

	DirectX::XMFLOAT4 result;
	DirectX::XMStoreFloat4(&result, rot);

	tran->SetWorldRotation(result);
}

void Camera::DrawInspector()
{
	ImGui::Checkbox(("MainCamera"), &mainCam);

	ImGui::DragFloat("FOV", &fov, 0.1f, 1.0f, 179.0f);
	ImGui::DragFloat("Near", &nearClip, 0.01f);
	ImGui::DragFloat("Far", &farClip, 1.0f);
	ImGui::DragFloat("Aspect", &aspect, 0.01f);
}

void Camera::Serialize(json& j) const
{
	j["Camera"] = mainCam;
}

void Camera::Deserialize(json& j)
{
	mainCam = j["Camera"];
}

std::unique_ptr<Component> Camera::Clone() const
{
	auto c = std::make_unique<Camera>();
	c->mainCam = false;

	return c;
}

