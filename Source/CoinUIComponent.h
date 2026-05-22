#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

class CoinUIComponent : public Component
{
public:
    CoinUIComponent() = default;
    ~CoinUIComponent() = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;
    void DrawInspector() override;

    // セーブ・ロード用の関数宣言
    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(CoinUIComponent)

private:
    int m_stageIndex = 0;
    int m_coinIndex = 0;
    int m_originalCol = -1;
};