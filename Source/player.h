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
    std::string normalPath = "./Data/Model/player/chara_motion.fbm/chameleon_TEX.png";
    std::string icePath = "./Data/Model/player/chara_motion.fbm/chameleon_tex_ice.png";
    std::string waterPath = "./Data/Model/player/chara_motion.fbm/chameleon_tex_water.png";
    std::string hotPath = "./Data/Model/player/chara_motion.fbm/chameleon_tex_fire.png";

    int currentTemp = 0;
};