#pragma once
#include <Component.h>
#include "Actor.h"
#include "nlohmann/json.hpp"
#include "System/Input.h"
#include "SpriteRender.h"

using json = nlohmann::json;

class TempDisplayController : public Component
{
public:
     TempDisplayController() = default;
    ~TempDisplayController() override = default;


    void Update(float elapsedTime) override;
    std::unique_ptr<Component> Clone() const override;
    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;
    void DrawInspector() override;
    void SetTemperature(int temp);

private:
    Actor* player = nullptr;

public:
    COMPONENT_ID(TempDisplayController)
};