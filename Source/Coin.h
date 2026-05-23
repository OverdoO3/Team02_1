#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class Coin : public Component
{
public:
    Coin() = default;
    ~Coin() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::string ToDataPath(const std::string& fullPath)
    {
        std::filesystem::path base = std::filesystem::absolute("Data");
        std::filesystem::path target = std::filesystem::absolute(fullPath);

        std::filesystem::path relative = std::filesystem::relative(target, base);

        std::filesystem::path normalized = relative.lexically_normal();

        return "Data/" + normalized.generic_string();
    }    

    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(Coin)


    bool GetIsCollected() const { return m_isCollected; }
    void SetIsCollected(bool isCollected) { m_isCollected = isCollected; }
private:
    int m_stageIndex = 0; 
    int m_coinIndex = 0;  
    bool m_isCollected = false;

    float m_jumpTimer = 0.0f;
    bool m_isAnimTimer = 0.0f;
    bool m_isAnimating = false;
    DirectX::XMFLOAT3 m_startPos;
    float m_jumpVelocityY = 0.0f;
    float m_spinAngle = 0.0f;
};