#include "CameraController.h"
#include "Actor.h"
#include "Scene.h"
#include "Factory.h"
#include "Lerp.h"
#include "SceneManager.h"
#include "RayCast.h"

//								↓に名前入れる
REGISTER_COMPONENT(ComponentID::CameraController , CameraController)

void CameraController::OnAwake(float elapsedTime)
{
	Actor* player = owner->GetScene()->FindByTag(1);
	if (player) {
		// 目標地点を原点(0,0,0)ではなく、最初からプレイヤーの位置にする
		focusTarget = player->GetComponent<Transform>()->GetWorldPosition();
		targetGoal = focusTarget;
	}
}

void CameraController::Update(float elapsedTime)
{
	Scene* scene = owner->GetScene();
	bool paused = scene && scene->sceneManager && scene->sceneManager->IsPaused();

	if (scene->isClear)
	{
		targetGoal = scene->FindByTag(3)->GetComponent<Transform>()->GetWorldPosition();
		focusTarget = Lerp(targetGoal, focusTarget, 0.1f);
		distance = Lerp(distance, 100.0f, 0.1f);
	}

	if (paused)
	{
		m_wasPaused = true;  // ポーズ中だったことを記録
		return;
	}

	// ポーズから復帰した最初のフレームはスキップ
	if (m_wasPaused)
	{
		m_wasPaused = false;
		// マウスをセンターに強制リセット
		if (mouseLocked)
		{
			HWND hwnd = GetActiveWindow();
			RECT rect;
			GetClientRect(hwnd, &rect);
			POINT center;
			center.x = rect.right / 2;
			center.y = rect.bottom / 2;
			ClientToScreen(hwnd, &center);
			SetCursorPos(center.x, center.y);
		}
		return;  // このフレームは処理しない
	}

	//if(InputC::KeyPressed(VK_TAB))
	//{
	//	mouseLocked = !mouseLocked;
	//	ShowCursor(!mouseLocked);
	//}
	
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
	Actor* playerModelChanger = owner->GetScene()->FindByTag(1);
	DirectX::XMFLOAT3 currentPos = owner->GetComponent<Transform>()->GetWorldPosition();
	auto transform = owner->GetComponent<Transform>();

	if (playerModelChanger)
	{
		auto transform = owner->GetComponent<Transform>();

		if (InputC::KeyPressed('E'))
		{
			focusToPlayer = !focusToPlayer;
		}
		targetGoal = focusToPlayer ?
			playerModelChanger->GetComponent<Transform>()->GetWorldPosition() :
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

		distance -= scroll * 3.0f; // 感度調整
		distance = std::clamp(distance, minDistance, maxDistance);

		transform->SetWorldPosition(cameraPos);
		transform->LookAt(focusTarget);
	}

	//if(Hit::RayCast(focusTarget,transform->GetWorldPosition(),))
}

void CameraController::DrawInspector()
{
	ImGui::InputFloat("Dist", &distance);
}

void CameraController::Serialize(nlohmann::json& j) const
{
	auto transform = owner->GetComponent<Transform>();
	DirectX::XMFLOAT3 pos = transform->GetWorldPosition();
	// JSONに今の座標を保存
	j["CameraPos"] = { pos.x, pos.y, pos.z };
}

void CameraController::Deserialize(nlohmann::json& j)
{
	if (j.contains("CameraPos"))
	{
		auto pos = j["CameraPos"];
		DirectX::XMFLOAT3 startPos = { pos[0], pos[1], pos[2] };
		// ロード時にこの位置をTransformに適用
		owner->GetComponent<Transform>()->SetWorldPosition(startPos);
	}
}

std::unique_ptr<Component> CameraController::Clone() const
{
	auto c = std::make_unique<CameraController>();

	c->distance = this->distance;
	c->pitch = this->pitch;
	c->yaw = this->yaw;
	c->maxPitch = this->maxPitch;
	c->minDistance = this->minDistance;
	c->maxDistance = this->maxDistance;
	c->focusToPlayer = this->focusToPlayer;

	return c;
}

void CameraController::UpdateCameraPosition()
{
	DirectX::XMFLOAT3 offset;
	offset.x = distance * cosf(pitch) * sinf(yaw);
	offset.y = distance * sinf(pitch);
	offset.z = distance * cosf(pitch) * cosf(yaw);

	Actor* player = owner->GetScene()->FindByTag(1);
	if (player) {
		targetGoal = focusToPlayer ? player->GetComponent<Transform>()->GetWorldPosition() : DirectX::XMFLOAT3{ 0,0,0 };
		focusTarget = targetGoal; // ここで初期値を代入しちゃう

		DirectX::XMFLOAT3 cameraPos = {
			focusTarget.x + offset.x,
			focusTarget.y + offset.y,
			focusTarget.z + offset.z
		};
		owner->GetComponent<Transform>()->SetWorldPosition(cameraPos);
		owner->GetComponent<Transform>()->LookAt(focusTarget);
	}
}