#pragma once
#include "Component.h"
#include "System/Sprite.h"
#include "OpenDialog.h"

class SpriteRender : public Component
{
public:
    COMPONENT_ID(SpriteRender)

    SpriteRender() {};
    ~SpriteRender() {};
    void Draw(RenderContext& rc) override;
    void Update(float elapsedTime) override;
    std::unique_ptr<Component> Clone() const override;

    Sprite* GetSprite(){ return spr.get(); }
    void SetSprite(std::unique_ptr<Sprite> sp) { spr = std::move(sp); }
    void SetString(const char* st) { texturepath = st; }

    void Serialize(nlohmann::json& j)const override;
    void Deserialize(nlohmann::json& j) override;

    void DrawInspector();

    std::string ToDataPath(const std::string& fullPath)
    {
        std::filesystem::path base = std::filesystem::absolute("Data");
        std::filesystem::path target = std::filesystem::absolute(fullPath);

        std::filesystem::path relative = std::filesystem::relative(target, base);

        std::filesystem::path normalized = relative.lexically_normal();

        return "Data/" + normalized.generic_string();
    }

private:
    std::unique_ptr<Sprite> spr;

    std::string texturepath;
};