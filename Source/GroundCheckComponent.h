#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"
#include "PhysicsSystem.h"

using json = nlohmann::json;

class GroundCheckComponent : public Component
{
public:
    GroundCheckComponent() = default;
    ~GroundCheckComponent() override = default;

    PhysicsSystem* physics;

    float rayLength = 3.0f;
    float offsetY = 0.5f;

    bool isGrounded = false;
    float groundY = 0.0f;
    DirectX::XMFLOAT3 groundNormal = { 0,1,0 };

    void Update(float elapsedTime)
    {
        CheckGround(elapsedTime);
    }

    bool IsGrounded() const { return isGrounded; }
    float GetGroundY() const { return groundY; }
    DirectX::XMFLOAT3 GetNormal() const { return groundNormal; }

    void DrawInspector() override {};

    void Serialize(nlohmann::json& j) const override {};
    void Deserialize(nlohmann::json& j) override {};

    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(GroundCheckComponent)

    void CheckGround(float elapsedTime);
};