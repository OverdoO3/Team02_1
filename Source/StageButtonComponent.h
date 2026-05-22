#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

class StageButtonComponent : public Component
{
public:

        void Update(float elapsedTime) override;
    void DrawInspector() override;
    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;
    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(StageButtonComponent)
private:
    int m_stageIndex = 0; // インスペクターで設定するID
};