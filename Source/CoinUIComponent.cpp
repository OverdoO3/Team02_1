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

    if (m_originalCol == -1) {
        m_originalCol = spr->GetTargetCol();
    }

    bool isCollected = scene->sceneManager->IsCoinCollected(m_stageIndex, m_coinIndex);

    // 見た目の計算
    int baseCol = spr->GetTargetCol();
    int splitX = spr->GetSplitX();
    if (splitX <= 0) splitX = 1;

    if (isCollected)
    {
        spr->SetTargetCol((m_originalCol + splitX - 1) % splitX);
    }
    else
    {
        spr->SetTargetCol(m_originalCol);
    }
}

void CoinUIComponent::DrawInspector()
{
    ImGui::InputInt("Stage Index (0~3)", &m_stageIndex);
    ImGui::InputInt("Coin Index (0~2)", &m_coinIndex);
}

void CoinUIComponent::Serialize(nlohmann::json& j) const
{
    j["StageIndex"] = static_cast<int>(m_stageIndex);
    j["CoinIndex"] = static_cast<int>(m_coinIndex);
}

void CoinUIComponent::Deserialize(nlohmann::json& j)
{
    m_stageIndex = (j.contains("StageIndex") && j["StageIndex"].is_number_integer()) ? j["StageIndex"].get<int>() : 0;
    m_coinIndex = (j.contains("CoinIndex") && j["CoinIndex"].is_number_integer()) ? j["CoinIndex"].get<int>() : 0;
}

std::unique_ptr<Component> CoinUIComponent::Clone() const
{
    auto c = std::make_unique<CoinUIComponent>();
    c->m_stageIndex = this->m_stageIndex;
    c->m_coinIndex = this->m_coinIndex;
    return c;
}