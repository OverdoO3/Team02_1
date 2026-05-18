#pragma once
#include "Component.h"
#include "System/Sprite.h"
#include "OpenDialog.h"
#include "ImGuizmo.h"
#include "CameraBase.h"
#include "Camera.h"


class SpriteRender : public Component
{
public:
    COMPONENT_ID(SpriteRender)

    SpriteRender() {};
    ~SpriteRender() {};
    void Draw(RenderContext& rc) override;
    void Update(float elapsedTime) override;
    std::unique_ptr<Component> Clone() const override;

    void SetAngle(float deg) { m_editorAngleDeg = deg; }
    void SetScale(float scale) { m_editorScale = scale; }

    Sprite* GetSprite(){ return spr.get(); }
    void SetSprite(std::unique_ptr<Sprite> sp) { spr = std::move(sp); }
    void SetString(const char* st) { texturepath = st; }

    void Serialize(nlohmann::json& j)const override;
    void Deserialize(nlohmann::json& j) override;

    void DrawInspector();
    int GetSortOrder() const { return sortOrder; }

    float GetSrcW() const { return m_srcW; }
    float GetSrcH() const { return m_srcH; }

    float GetEditorScale() const { return m_editorScale; }
    float GetSrcX() const { return m_srcX; }
    float GetSrcY() const { return m_srcY; }

    void SetColor(DirectX::XMFLOAT4 color)
    {
        this->color = color;
    }

    std::string ToDataPath(const std::string& fullPath)
    {
        std::filesystem::path base = std::filesystem::absolute("Data");
        std::filesystem::path target = std::filesystem::absolute(fullPath);

        std::filesystem::path relative = std::filesystem::relative(target, base);

        std::filesystem::path normalized = relative.lexically_normal();

        return "Data/" + normalized.generic_string();
    }

    enum class Pivot { TopLeft, Center };
    Pivot m_pivot = Pivot::TopLeft;

private:
    std::unique_ptr<Sprite> spr;

    std::string texturepath;

    float m_editorScale = 1.0f;
    float m_editorAngleDeg = 0.0f;

    //切り抜きサイズ
    float m_srcX = 0.0f;
    float m_srcY = 0.0f;
    float m_srcW = -1.0f;
    float m_srcH = -1.0f;
    int sortOrder = 0;
    int m_splitX = 1;
    int m_splitY = 1;
    int m_spriteIndex = 0;
    int m_targetCol = 0;
    int m_targetRow = 0;
    bool m_isLoop = false;
    float m_frameDuration = 0.1f;
    float m_timer = 0.0f;
    int m_animFrameCount = 1;
    int m_currentFrame = 0;

    bool m_mouseWasDown = false;

    //デバッグ矩形用(当たり判定も入れてます)
    float m_lastDrawX = 0.0f;  // 実際に描画した左上X
    float m_lastDrawY = 0.0f;  // 実際に描画した左上Y
    float m_lastDrawW = 0.0f;
    float m_lastDrawH = 0.0f;

    float m_colliderOffsetX = 0.0f; // 判定の横ズレ
    float m_colliderOffsetY = 0.0f; // 判定の縦ズレ
    float m_colliderWidth = 100.0f; // 判定の横幅
    float m_colliderHeight = 100.0f; // 判定の縦幅

    
    bool m_showCollider = true;


    //色
    DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
};