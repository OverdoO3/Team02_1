#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class firewood : public Component
{
public:
    firewood() = default;
    ~firewood() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    bool GetFire() const { return onfire; }

    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(firewood)
private:
    bool onfire = true;
};