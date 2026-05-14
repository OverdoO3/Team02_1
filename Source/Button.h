#pragma once
#include <Component.h>
#include "Actor.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class Button : public Component
{
public:
    Button() = default;
    ~Button() override = default;

    void Update(float elapsedTime) override;

    void OnAwake(float elapsedTime) override;

    std::unique_ptr<Component> Clone() const override;
    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    void DrawInspector() override;

private:
    bool IsMouseOver();
    void OnClick();

    std::string m_nextSceneName;
public:
    COMPONENT_ID(Button)
};