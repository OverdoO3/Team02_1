#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class ThermalBody : public Component
{
public:
    ThermalBody() = default;
    ~ThermalBody() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void SetHeat(int temp) { temperature = temp; }
    void AddHeat(int temp) { temperature += temp; }
    int GetHeat() { return temperature; }

    void DrawInspector() override;
    void RenderDebug(RenderContext& rc, ShapeRenderer* renderer);

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    float GetRadius() { return radius; }
    void GetRadius(float radius) { this->radius = radius; }

    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(ThermalBody)
private:
    int temperature = 0;

    float radius = 5.0f;

    DirectX::XMFLOAT4 color;
};