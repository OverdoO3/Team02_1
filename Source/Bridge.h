#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class Bridge : public Component
{
public:
    Bridge() = default;
    ~Bridge() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(Bridge)
private:
    float saveY;
    float deathY;

    bool death = false;
    float timer = 1.0f;
};