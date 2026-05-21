#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class playerModelChanger : public Component
{
public:
    playerModelChanger() = default;
    ~playerModelChanger() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(playerModelChanger)
private:
    std::string normalPath = "./Data/Model/player/chara_motion.mdl";
    std::string icePath = "./Data/Model/player/chara_motion_ice.mdl";
    std::string waterPath = "./Data/Model/player/chara_motion_water.mdl";
    std::string hotPath = "./Data/Model/player/chara_motion_fire.mdl";

    int currentTemp = 0;
};