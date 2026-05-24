#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class snowman : public Component
{
public:
    snowman() = default;
    ~snowman() override = default;

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

    COMPONENT_ID(snowman)
private:
    bool death;
    float timer = 1.4f;
};