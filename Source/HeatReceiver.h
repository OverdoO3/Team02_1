#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"
#include "System/Graphics.h"

class Actor;
class ThermalBody;

using json = nlohmann::json;
class HeatReceiver : public Component
{
public:
    HeatReceiver() = default;
    ~HeatReceiver() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;

    void SetHeatNum(int heat);

    float GetRadius() { return radius; }

    float GetHeatNum() { return heatNum; }

    bool IsPlayerNear() const { return m_isPlayerNear; }

    bool IsWasInside() const { return m_wasInside; }
    void SetWasInside(bool isInside) { m_wasInside = isInside; }

    std::string ToDataPath(const std::string& fullPath)
    {
        std::filesystem::path base = std::filesystem::absolute("Data");
        std::filesystem::path target = std::filesystem::absolute(fullPath);

        std::filesystem::path relative = std::filesystem::relative(target, base);

        std::filesystem::path normalized = relative.lexically_normal();

        return "Data/" + normalized.generic_string();
    }


    COMPONENT_ID(HeatReceiver)
private:
    int heatNum;
    int m_linkedLightIndex = -1;

    float radius = 5.0f;

    ThermalBody* m_playerThermal = nullptr; // プレイヤーのThermalBodyをキャッシュ
    bool m_isPlayerNear = false;           // プレイヤーが近くにいるかのフラグ
    bool m_wasInside = false;
};