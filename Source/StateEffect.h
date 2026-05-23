#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"
#include "Effect.h"
#include <Effekseer.h>

using json = nlohmann::json;
class StateEffect : public Component
{
public:
    StateEffect() = default;
    ~StateEffect() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    void SetState(const std::string& state);

    std::string ToDataPath(const std::string& fullPath)
    {
        std::filesystem::path base = std::filesystem::absolute("Data");
        std::filesystem::path target = std::filesystem::absolute(fullPath);

        std::filesystem::path relative = std::filesystem::relative(target, base);

        std::filesystem::path normalized = relative.lexically_normal();

        return "Data/" + normalized.generic_string();
    }

    std::unique_ptr<Component> Clone() const override;

    COMPONENT_ID(StateEffect)
private:
    struct StateData
    {
        std::string effectPath;
        std::shared_ptr<Effect> effect;
        bool isFirstState = false;
    };
    std::unordered_map<std::string, StateData> states;

    std::string currentState;
    Effekseer::Handle handle = -1;

    bool loop = false;
};