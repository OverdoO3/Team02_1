#include "ButtonComponent.h"
#include "Factory.h"
#include "Scene.h"
#include "SceneManager.h"

//								↓に名前入れる
REGISTER_COMPONENT(ComponentID::ButtonComponent, ButtonComponent)


void ButtonComponent::Update(float elapsedTime)
{
    // マウスがボタンの上にあるか確認
    if (!owner)return;
    auto spr = owner->GetComponent<SpriteRender>();
    if (spr)
    {
        bool hovering = spr->IsHovered();
        if (hovering)
        {
            spr->SetColor(DirectX::XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f));

            auto& gp = Input::Instance().GetGamePad();
            if (gp.IsButtonDown(gp.BTN_LEFT))
            {
                OnClick();
            }
            if (GetAsyncKeyState(VK_LBUTTON) & 1)  // & 1 で押した瞬間だけ
            {
                OnClick();
            }
        }
        else
        {
            spr->SetColor(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
        }
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
    if (m_nextSceneName.empty())
    {
        // インスペクターでパスが空欄のままボタンが押された場合
        LogManager::Instance().AddLog(
            LogCategory::scene,
            LogEvent::Scene_Transition,
            " [Error] NextSceneName is Empty! (Actor: " + owner->GetName() + ")"
        );
        return;
    }

    Scene* currentScene = owner->GetScene();
    if (!currentScene)return;

    //シーンが持ってるSceneManagerのポインタを取得
    SceneManager* sceneManager = currentScene->sceneManager;
    if (sceneManager)
    {
        //次のシーンオブジェクトを生成
        auto nextSceneInstance = std::make_unique<Scene>();

        //シーン遷移を実行(インスペクターで指定したJSONパスを渡す)
        sceneManager->ChangeScene(std::move(nextSceneInstance), m_nextSceneName.c_str());

    }
}