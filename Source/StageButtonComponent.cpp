#include "StageButtonComponent.h"
#include "Actor.h"
#include "Scene.h"
#include "SceneManager.h"
#include "SpriteRender.h"

REGISTER_COMPONENT(ComponentID::StageButtonComponent, StageButtonComponent)

void StageButtonComponent::Update(float elapsedTime)
{
    auto* scene = owner->GetScene();
    if (!scene || !scene->sceneManager) return;

    auto* spr = owner->GetComponent<SpriteRender>();
    if (!spr) return;

    // マウスが乗っているなら、SceneManagerの掲示板に自分のIDを書き込む
    if (spr->IsHovered())
    {
        scene->sceneManager->SetHoveredStage(m_stageIndex);
    }

    else if (scene->sceneManager->GetHoveredStage() == m_stageIndex)
    {
        scene->sceneManager->SetHoveredStage(-1);
    }
}

void StageButtonComponent::DrawInspector()
{
    ImGui::InputInt("Target Stage Index", &m_stageIndex);
}

void StageButtonComponent::Serialize(nlohmann::json& j) const
{
    j["StageIndex"] = m_stageIndex;
}

void StageButtonComponent::Deserialize(nlohmann::json& j)
{
    m_stageIndex = j.value("StageIndex", 0);
}

std::unique_ptr<Component> StageButtonComponent::Clone() const
{
    auto c = std::make_unique<StageButtonComponent>();
    c->m_stageIndex = this->m_stageIndex;
    return c;
}