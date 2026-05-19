#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"
#include "Effect.h"
#include "EffectManager.h"

using json = nlohmann::json;
class EffectRender : public Component
{
public:
    EffectRender() = default;
    ~EffectRender() override = default;

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

    void Play();
    void Stop();
    void OnDestroy();

    void SetEffect(const std::shared_ptr<Effect>& eff);
    void SetScale(float scale);

    COMPONENT_ID(EffectRender)
private:
    std::shared_ptr<Effect> effect;
    Effekseer::Handle handle = -1;
    std::string effectPath;

    DirectX::XMFLOAT3 position{0,0,0};
    float scale;

    bool playOnStart = true;
    bool loop;
};