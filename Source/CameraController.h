#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

//     ComponentManagerのEnumの中身を増やす
//　　↓作ったら名前変える
class CameraController : public Component
{
public:
    CameraController() = default;
    ~CameraController() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(CameraController)
private:
    float distance = 100;
    float yaw = 0.0f;
    float pitch = 20.0f;
    float maxPitch = 1.3f; //上下の上限
    float minDistance = 40.0f; //どれくらい近づけれるか
    float maxDistance = 300.0f; //どれくらい遠ざかれるか

    bool focusToPlayer = false;

    DirectX::XMFLOAT3 focusTarget;   // 今見てる中心
    DirectX::XMFLOAT3 targetGoal;    // 目標の中心
};