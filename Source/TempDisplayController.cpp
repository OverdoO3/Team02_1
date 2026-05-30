#include "TempDisplayController.h"
#include "Actor.h"
#include "SpriteRender.h"

REGISTER_COMPONENT(ComponentID::TempDisplayController, TempDisplayController)


// 毎フレームのUpdate処理は不要なので空にします
void TempDisplayController::Update(float elapsedTime)
{
}

// 外から温度を渡して画像を切り替える関数
void TempDisplayController::SetTemperature(int temp)
{
    auto sr = owner->GetComponent<SpriteRender>();
    if (!sr) return;

    if (temp == 1)       sr->SetTargetCol(0); // 炎
    else if (temp == -1) sr->SetTargetCol(1); // 水
    else if (temp == -2) sr->SetTargetCol(2); // 氷
}

std::unique_ptr<Component> TempDisplayController::Clone() const {
    return std::make_unique<TempDisplayController>(*this);
}

void TempDisplayController::Serialize(nlohmann::json& j) const {}
void TempDisplayController::Deserialize(nlohmann::json& j) {}

void TempDisplayController::DrawInspector()
{
    // テストボタン
    if (ImGui::Button("Test Flame (Col 0)")) { SetTemperature(1); }
    if (ImGui::Button("Test Water (Col 1)")) { SetTemperature(-1); }
    if (ImGui::Button("Test Ice   (Col 2)")) { SetTemperature(-2); }
}