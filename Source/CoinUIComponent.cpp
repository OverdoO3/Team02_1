#include "CoinUIComponent.h"
#include "Actor.h"
#include "Scene.h"
#include "SceneManager.h"
#include "SpriteRender.h"
#include "Coin.h"

REGISTER_COMPONENT(ComponentID::CoinUIComponent, CoinUIComponent)

void CoinUIComponent::OnAwake(float elapsedTime)
{

}

void CoinUIComponent::Update(float elapsedTime)
{
    auto* scene = owner->GetScene();
    if (!scene || !scene->sceneManager) return;
    auto* spr = owner->GetComponent<SpriteRender>();
    if (!spr) return;

    // --- 1. 共通の初期化 ---
    if (m_originalCol == -1) {
        m_originalCol = spr->GetTargetCol();
    }

    // --- 2. 【ゲームモード用】コイン取得フラグによるスプライトずらし ---
    // ステージ選択モードじゃない時、もしくは両方動かしたい時でも動くようにする
    bool isCollected = scene->sceneManager->IsCoinCollected(m_stageIndex, m_coinIndex);
    int splitX = spr->GetSplitX();
    if (splitX <= 0) splitX = 1;

    if (isCollected) {
        spr->SetTargetCol((m_originalCol + splitX - 1) % splitX);
    }
    else {
        spr->SetTargetCol(m_originalCol);
    }

    // --- 3. 【ステージ選択用】アルファフェード処理 ---
    // このモードのときだけ、アルファを操作する
    if (m_isStageSelectMode)
    {
        int hoveredStage = scene->sceneManager->GetHoveredStage();
        float targetAlpha = (hoveredStage == m_stageIndex) ? 1.0f : 0.0f;

        float t = 7.0f * elapsedTime;
        m_appearanceRatio += (targetAlpha - m_appearanceRatio) * t;
        m_appearanceRatio = std::clamp(m_appearanceRatio, 0.0f, 1.0f);

        DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f }; 
        color.w = m_appearanceRatio;
        spr->SetColor(color);
    }
    else
    {
        // ステージ選択モードじゃない時は、アルファを強制的に1.0にしておく
        // (これをしないと、前のシーンから戻ってきた時に透明なままになる可能性があるため)
        DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
        color.w = 1.0f;
        spr->SetColor(color);
    }
}


void CoinUIComponent::DrawInspector()
{
    ImGui::Checkbox("Is Stage Select Mode", &m_isStageSelectMode);

    ImGui::InputInt("Stage Index", &m_stageIndex);

    if (m_isStageSelectMode)
    {
        // ステージ選択モードなら、特別な追加設定があればここに書く
        ImGui::Text("Mode: Stage Select (Hover-linked)");
    }
    else
    {
        // 通常のコインUIモードなら CoinIndex を表示
        ImGui::InputInt("Coin Index", &m_coinIndex);
    }
}

void CoinUIComponent::Serialize(nlohmann::json& j) const
{
    j["StageIndex"] = static_cast<int>(m_stageIndex);
    j["CoinIndex"] = static_cast<int>(m_coinIndex);
    j["StageSelectMode"] = m_isStageSelectMode;
}

void CoinUIComponent::Deserialize(nlohmann::json& j)
{
    m_stageIndex = (j.contains("StageIndex") && j["StageIndex"].is_number_integer()) ? j["StageIndex"].get<int>() : 0;
    m_coinIndex = (j.contains("CoinIndex") && j["CoinIndex"].is_number_integer()) ? j["CoinIndex"].get<int>() : 0;
    m_isStageSelectMode = j.value("StageSelectMode", false);
}

std::unique_ptr<Component> CoinUIComponent::Clone() const
{
    auto c = std::make_unique<CoinUIComponent>();
    c->m_stageIndex = this->m_stageIndex;
    c->m_coinIndex = this->m_coinIndex;
    c->m_isStageSelectMode = this->m_isStageSelectMode;
    return c;
}