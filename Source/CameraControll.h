#pragma once
#include "Component.h"
#include "Context.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class CameraController : public Component
{
public:
    CameraController() = default;
    ~CameraController() override = default;

    void Update(Context context) override;
    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(CameraController)
};