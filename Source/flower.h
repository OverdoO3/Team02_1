#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class flower : public Component
{
public:
    flower() = default;
    ~flower() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(flower)
private:
    std::string openPath = "./Data/Model/Stage/Demos/flower_obj.mdl";
    std::string clocsePath = "./Data/Model/Stage/Demos/bud_obj.mdl";

    float saveY;
    float deathY;

    bool death;
    float timer = 1.0f;

    bool isOpen;
    bool once = false;
    bool once2 = false;
};