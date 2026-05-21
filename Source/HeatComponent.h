#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class HeatTransfer : public Component
{
public:
    HeatTransfer() = default;
    ~HeatTransfer() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;
    std::vector<Actor*> m_insideActors;
    const std::vector<Actor*>& GetInsideActors() const { return m_insideActors; }

    COMPONENT_ID(HeatTransfer)
private:
};