#include "CameraController.h"
#include "Actor.h"
#include "Scene.h"
#include "Factory.h"
#include "Lerp.h"

//								↓に名前入れる
REGISTER_COMPONENT(ComponentID::CameraController , CameraController)

void CameraController::OnAwake(float elapsedTime)
{
}

void CameraController::Update(float elapsedTime)
{
	if(InputC::KeyPressed(VK_TAB))
	{
		mouseLocked = !mouseLocked;
		ShowCursor(!mouseLocked);
	}
	
	if (mouseLocked)
	{
		POINT pos;
		GetCursorPos(&pos);

		static POINT prev = pos;

		POINT center;
		RECT rect;
		HWND hwnd = GetActiveWindow();

		GetClientRect(hwnd, &rect);

		center.x = rect.right / 2;
		center.y = rect.bottom / 2;

		ClientToScreen(hwnd, &center);

		// 差分
		float deltaX = pos.x - center.x;
		float deltaY = pos.y - center.y;

		// 戻す
		SetCursorPos(center.x, center.y);

		prev = pos;

		yaw += deltaX * 0.01f;
		pitch += deltaY * 0.01f;

	}
	// 上下制限（めっちゃ重要）
	pitch = std::clamp(pitch, -maxPitch, maxPitch);

	DirectX::XMFLOAT3 offset;

	offset.x = distance * cosf(pitch) * sinf(yaw);
	offset.y = distance * sinf(pitch);
	offset.z = distance * cosf(pitch) * cosf(yaw);

	DirectX::XMFLOAT3 target;
	Actor* player = owner->GetScene()->FindByTag(1);
	DirectX::XMFLOAT3 currentPos = owner->GetComponent<Transform>()->GetWorldPosition();
	auto transform = owner->GetComponent<Transform>();

	if (player)
	{
		auto transform = owner->GetComponent<Transform>();

		if (InputC::KeyPressed('E'))
		{
			focusToPlayer = !focusToPlayer;
		}
		targetGoal = focusToPlayer ?
			player->GetComponent<Transform>()->GetWorldPosition() :
			DirectX::XMFLOAT3{ 0,0,0 };

		// スムーズに中心移動
		focusTarget = Lerp(focusTarget, targetGoal, 0.1f);

		// カメラ位置
		DirectX::XMFLOAT3 cameraPos = {
			focusTarget.x + offset.x,
			focusTarget.y + offset.y,
			focusTarget.z + offset.z
		}; 
		float scroll = ImGui::GetIO().MouseWheel;

		distance -= scroll * 1.0f; // 感度調整
		distance = std::clamp(distance, minDistance, maxDistance);

		transform->SetWorldPosition(cameraPos);
		transform->LookAt(focusTarget);
	}
}

void CameraController::DrawInspector()
{
	ImGui::InputFloat("Dist", &distance);
}

void CameraController::Serialize(nlohmann::json& j) const
{
}

void CameraController::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> CameraController::Clone() const
{
	auto c = std::make_unique<CameraController>();
	return c;
}
