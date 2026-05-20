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
        OnClick();
    }
}

void ButtonComponent::DrawInspector()
{
    // モード選択コンボボックス
    const char* modeNames[] = {
        "Scene Change",
        "Quit Game",
        "Resume Game (Pause)",
        "Restart Scene",
        "Return to Title",
    };
    int currentMode = static_cast<int>(m_buttonMode);
    if (ImGui::Combo("Button Mode", &currentMode, modeNames, IM_ARRAYSIZE(modeNames)))
    {
        m_buttonMode = static_cast<ButtonMode>(currentMode);
    }

    // モードに応じて追加設定を表示
    switch (m_buttonMode)
    {
    case ButtonMode::SceneChange:
    {
        char buf[256];
        strcpy_s(buf, m_nextSceneName.c_str());
        if (ImGui::InputText("Next Scene JSON Path", buf, sizeof(buf)))
        {
            m_nextSceneName = buf;
        }
        ImGui::Text("Example: Scenes/Demo2.json");
        break;
    }
    case ButtonMode::ReturnToTitle:
    {
        char buf[256];
        strcpy_s(buf, m_titleSceneName.c_str());
        if (ImGui::InputText("Title Scene JSON Path", buf, sizeof(buf)))
        {
            m_titleSceneName = buf;
        }
        ImGui::Text("Example: Scenes/Title.json");
        break;
    }
    default:
        break;
    }
}

void ButtonComponent::Serialize(nlohmann::json& j) const
{
    j["NextSceneName"] = m_nextSceneName;
    j["TitleSceneName"] = m_titleSceneName;
    j["ButtonMode"] = static_cast<int>(m_buttonMode);
}


void ButtonComponent::Deserialize(nlohmann::json& j)
{
    m_nextSceneName = j.value("NextSceneName", "");
    m_titleSceneName = j.value("TitleSceneName", "Scenes/Title.json");

    if (j.contains("ButtonMode"))
    {
        m_buttonMode = static_cast<ButtonMode>(j.value("ButtonMode", 0));
    }
    else if (j.value("IsQuitButton", false))
    {
        m_buttonMode = ButtonMode::QuitGame;
    }
    else
    {
        m_buttonMode = ButtonMode::SceneChange;
    }
}


std::unique_ptr<Component> ButtonComponent::Clone() const
{
    auto c = std::make_unique<ButtonComponent>();
    c->m_nextSceneName = this->m_nextSceneName;
    c->m_titleSceneName = this->m_titleSceneName;
    c->m_buttonMode = this->m_buttonMode;
    return c;
}

void ButtonComponent::OnClick()
{
    if (!owner) return;

    Scene* scene = owner->GetScene();
    if (!scene) return;
    SceneManager* sm = scene->sceneManager;
    if (!sm) return;

    switch (m_buttonMode)
    {
        // ── ゲーム終了 ────────────────────────────────────────────
    case ButtonMode::QuitGame:
        PostQuitMessage(0);
        return;

        // ── ゲームに戻る（ポーズ解除） ────────────────────────────
    case ButtonMode::ResumeGame:
        sm->SetPause(false);
        return;

        // ── やり直す（現在シーンをリロード） ──────────────────────
    case ButtonMode::RestartScene:
        sm->RequestSceneChange(sm->GetCurrentScenePath());
        return;

        // ── タイトルに戻る ────────────────────────────────────────
    case ButtonMode::ReturnToTitle:
        sm->SetPause(false);   // ポーズ中でも確実に解除してから遷移
        sm->RequestSceneChange(m_titleSceneName);
        return;

        // ── シーン遷移（既存） ────────────────────────────────────
    case ButtonMode::SceneChange:
    default:
        if (m_nextSceneName.empty()) return;
        sm->RequestSceneChange(m_nextSceneName);
        return;
    }
}