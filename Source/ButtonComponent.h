#pragma once
#include <Component.h>
#include "Actor.h"
#include "nlohmann/json.hpp"
#include "System/Input.h"
using json = nlohmann::json;

class ButtonComponent : public Component
{
public:
    ButtonComponent() = default;
    ~ButtonComponent() override = default;

    // ボタンの動作モード
    enum class ButtonMode
    {
        SceneChange,    // シーン遷移（既存）
        QuitGame,       // ゲーム終了
        ResumeGame,     // ゲームに戻る（ポーズ解除）
        RestartScene,   // やり直す（現在シーンをリロード）
        ReturnToTitle,  // タイトルに戻る
    };

    void Update(float elapsedTime) override;
    std::unique_ptr<Component> Clone() const override;
    bool IsMouseOver();
    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;
    void DrawInspector() override;

    bool GetIsPauseButton() const
    {
        return m_buttonMode == ButtonMode::ResumeGame ||
            m_buttonMode == ButtonMode::RestartScene ||
            m_buttonMode == ButtonMode::ReturnToTitle;
    }

private:
    void OnClick();
    bool m_clickedThisFrame = false;
    std::string m_nextSceneName = "";
    bool m_isQuitButton = false;          

    ButtonMode m_buttonMode = ButtonMode::SceneChange;
    std::string m_titleSceneName = "Scenes/title.json";

public:
    COMPONENT_ID(ButtonComponent)
};