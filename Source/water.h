#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class water : public Component
{
public:
    water() = default;
    ~water() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(water)
private:
    std::string waterpath = "Data/Model/Stage/Demos/water_block.mdl";
    std::string icepath = "Data/Model/Stage/Demos/ice_block.mdl";
};