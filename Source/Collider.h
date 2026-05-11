#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct AABB
{
    DirectX::XMFLOAT3 min;
    DirectX::XMFLOAT3 max;

    bool Intersects(const AABB& other) const
    {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
            (min.y <= other.max.y && max.y >= other.min.y) &&
            (min.z <= other.max.z && max.z >= other.min.z);
    }
};

class Collider : public Component
{
public:
    Collider() {};
    ~Collider() override = default;

    virtual AABB GetAABB() const = 0;

    void Update(float elapsedTime) override {};
    void OnAwake(float elapsedTime) override;
    virtual void DrawInspector() = 0;

    virtual void RenderDebug(RenderContext& rc, ShapeRenderer* shapeRenderer) = 0;

    virtual void Serialize(nlohmann::json& j) const = 0;
    virtual void Deserialize(nlohmann::json& j) = 0;

    virtual std::unique_ptr<Component> Clone() const = 0;
};

struct Ray
{
    DirectX::XMFLOAT3 origin;
    DirectX::XMFLOAT3 direction; // ê≥ãKâªëOíÒ
};

struct RaycastHit
{
    Collider* collider = nullptr;
    DirectX::XMFLOAT3 point;
    DirectX::XMFLOAT3 normal;
    float distance = 0.0f;
};
