#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Move : public Component
{
public:
    COMPONENT_ID(Move)

       Move() = default;
    ~Move() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;
    void DrawInspector() override;

    void RenderDebug(RenderContext& rc, ShapeRenderer* shapeRenderer) override;
    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    void ChangeModel();

    std::string ToDataPath(const std::string& fullPath)
    {
        std::filesystem::path base = std::filesystem::absolute("Data");
        std::filesystem::path target = std::filesystem::absolute(fullPath);

        std::filesystem::path relative = std::filesystem::relative(target, base);

        std::filesystem::path normalized = relative.lexically_normal();

        return "Data/" + normalized.generic_string();
    }

    std::unique_ptr<Component> Clone() const override;
private:
    DirectX::XMFLOAT3 Velocity{ 0,0,0 };
    float speed = 30.0f;
    float turnSpeed = 5.0f;
    float grav = 4.0f;

    bool OnGround;
    float downhillOffset = 1.0f;

    float radius = 1.5f;
    float height = 1.5f;
    float stepHeight = 0.4f;

    enum AnimState
    {
        idle,
        walk,
        in_out,
        land,
        fall,
        goal,
    };

    AnimState currentState = AnimState::idle;
    AnimState nextState;

    float m_goalTimer = 0.0f;
    bool m_isTransitionStarted = false;
};