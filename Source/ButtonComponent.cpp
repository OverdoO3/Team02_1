#include "ButtonComponent.h"
#include "Factory.h"
#include "Scene.h"
#include "SceneManager.h"

REGISTER_COMPONENT(ComponentID::ButtonComponent, ButtonComponent)

void ButtonComponent::Update(float elapsedTime)
{
    if (!owner) return;
    auto spr = owner->GetComponent<SpriteRender>();
    if (spr)
    {
        bool hovering = spr->IsHovered();
        if (hovering)
        {
            spr->SetColor(DirectX::XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f));
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                m_clickedThisFrame = true;
            }
        }
        else
        {
            spr->SetColor(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
        }
    }

    if (m_clickedThisFrame)
    {
        m_clickedThisFrame = false;
        Scene* currentScene = owner->GetScene();
        if (!currentScene) return;
        SceneManager* sceneManager = currentScene->sceneManager;
        if (!sceneManager) return;
        sceneManager->RequestSceneChange(m_nextSceneName);
        return;
    }
}

void ButtonComponent::DrawInspector()
{
    char buf[256];
    strcpy_s(buf, m_nextSceneName.c_str());
    if (ImGui::InputText("Next Scene JSON Path", buf, sizeof(buf)))
    {
        m_nextSceneName = buf;
    }
    ImGui::Text("Example: Scenes/Demo2.json");
}

void ButtonComponent::Serialize(nlohmann::json& j) const
{
    j["NextSceneName"] = m_nextSceneName;
}

void ButtonComponent::Deserialize(nlohmann::json& j)
{
    m_nextSceneName = j.value("NextSceneName", "");
}

std::unique_ptr<Component> ButtonComponent::Clone() const
{
    auto c = std::make_unique<ButtonComponent>();
    c->m_nextSceneName = this->m_nextSceneName;
    return c;
}

void ButtonComponent::OnClick()
{
    if (!owner) return;
    if (m_nextSceneName.empty()) return;
    Scene* currentScene = owner->GetScene();
    if (!currentScene) return;
    SceneManager* sceneManager = currentScene->sceneManager;
    if (!sceneManager) return;
    sceneManager->RequestSceneChange(m_nextSceneName);
}