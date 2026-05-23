#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class HeatTransfer : public Component
{
public:
    HeatTransfer() = default;
    ~HeatTransfer() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;
    std::vector<Actor*> m_insideActors;
    const std::vector<Actor*>& GetInsideActors() const { return m_insideActors; }
    bool GetCanAbsorb() const { return m_canAbsorb; }

    std::string ToDataPath(const std::string& fullPath)
    {
        std::filesystem::path base = std::filesystem::absolute("Data");
        std::filesystem::path target = std::filesystem::absolute(fullPath);

        std::filesystem::path relative = std::filesystem::relative(target, base);

        std::filesystem::path normalized = relative.lexically_normal();

        return "Data/" + normalized.generic_string();
    }

    bool IsStatusActive() const;
    COMPONENT_ID(HeatTransfer)
private:
    bool m_canAbsorb = false;
    int m_prevHeat = 0;
};