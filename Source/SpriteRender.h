#pragma once
#include "Component.h"
#include "System/Sprite.h"
#include "OpenDialog.h"
#include "ImGuizmo.h"
#include "CameraBase.h"
#include "Camera.h"
#include"HeatComponent.h"

class Actor;
class SceneManager;
class ThermalBody;

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

    Sprite* GetSprite() { return spr.get(); }
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

    void SetSceneManager(SceneManager* sm) { m_sceneManager = sm; }

    bool IsHovered()const { return m_isHovered; }

    std::string ToDataPath(const std::string& fullPath)
    {
        std::filesystem::path base = std::filesystem::absolute("Data");
        std::filesystem::path target = std::filesystem::absolute(fullPath);

        std::filesystem::path relative = std::filesystem::relative(target, base);

        std::filesystem::path normalized = relative.lexically_normal();

        return "Data/" + normalized.generic_string();
    }

    bool GetIsPauseUI()const { return m_isPauseUI; }

    enum class Pivot { TopLeft, Center };
    Pivot m_pivot = Pivot::TopLeft;


    enum class IrisMode { None, Out, In };

    void StartIrisOut();

    void StartIrisIn();

    IrisMode GetIrisMode() const { return m_irisMode; }
    void StartPopUp(float duration = 0.4f, float maxScale = 1.5f);

    bool GetUsePopUpClear()const { return m_usePopUpClear; }
    float GetPopUpDuration()const { return m_popUpDuration; }
    float GetMaxPopScale() const { return m_maxPopScale; }

    int GetTargetCol()const { return m_targetCol; }
    int GetTargetRow()const { return m_targetRow; }

    int GetSplitX()const { return m_splitX; }
    int GetSplitY()const { return m_splitY; }

    void SetSrcX(float srcX) { m_srcX = srcX; }
    void SetSrcY(float srcY) { m_srcY = srcY; }
    void SetTargetCol(int col) { m_targetCol = col; }

    bool IsClickCompleted()const
    {
        if (!m_useClickHide || m_clickCountLimit <= 0) return true;
        return m_currentClickCount >= m_clickCountLimit;
    }
    bool IsClickRestrictionActive() const {
        return m_useClickHide && m_clickCountLimit > 0 && m_currentClickCount < m_clickCountLimit;
    }

    void UpdateClickLogic();

private:
    std::unique_ptr<Sprite> spr;

    std::string texturepath;
    SceneManager* m_sceneManager = nullptr;

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
    bool m_isHovered = false;

    bool m_isPauseUI = false;

    //色
    DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };

    bool  m_hoverFade = false;       // インスペクターでホバーでフェード出現させるかのスイッチ
    float m_appearanceRatio = 0.0f;  // 現在の出現割合 (0.0f = 完全に透明/消滅、1.0f = 完全に表示)
    float m_fadeSpeed = 5.0f;        // フェードの速さ

    bool m_hoverSpriteShift = false;
    int m_hoverCollOffset = 1;
    int m_hoverRowOffset = 0;

    bool  m_isIrisActive = false;

    IrisMode m_irisMode = IrisMode::None;
    float m_irisDuration = 1.0f;
    float m_irisTargetScale = 0.0f;
    float m_irisMaxScale = 150.0f;

    float m_irisTimer = 0.0f;
    float m_irisStartScale = 1.0f;
    float m_originalScale = 1.0f;

    bool m_isGameLoading = false;  // ゲームに行くときのLoadingで出すか
    bool m_isTitleLoading = false; // タイトルに戻るときのLoadingで出すか
    bool m_isOnlyWhileLoading = false;

    // 揺れ設定
    bool m_swingEnabled = false;
    bool m_useSwingX = false;   // X方向（sin/cos）
    bool m_useSwingY = false;   // Y方向（sin/cos）
    bool m_swingXUseCos = false; // trueでcos、falseでsin
    bool m_swingYUseCos = true;  // trueでcos、falseでsin
    float m_swingAmplitudeX = 10.0f;  // X振幅
    float m_swingAmplitudeY = 10.0f;  // Y振幅
    float m_swingSpeedX = 1.0f;       // X速度
    float m_swingSpeedY = 1.0f;       // Y速度
    float m_swingOffsetX = 0.0f;      // X位相オフセット
    float m_swingOffsetY = 0.0f;      // Y位相オフセット
    float m_swingTimer = 0.0f;

    // 回転揺れ
    bool m_useSwingRot = false;
    float m_swingRotAmplitude = 5.0f;
    float m_swingRotSpeed = 1.0f;
    bool m_swingRotUseCos = false;

    bool m_isFadeEnabled = false;
    float m_sprFadeSpeed = 2.0f;
    float m_fadeTimer = 0.0f;

    float m_fadeAlpha = 1.0f;

    bool m_useClickHide = false;
    bool m_isClickedHidden = false;

    bool m_isPopUp = false;
    float m_popUpTimer = 0.0f;
    float m_popUpDuration = 0.4f;
    float m_baseScale = 1.0f;
    float m_maxPopScale = 1.5f;

    bool m_usePopUpClear = false;
    bool m_isOnlyWhileGoal = false;

    bool m_useHeatUI = false;
    bool m_showAbnormal = false;

    bool m_isScaleLoopEnabled = false;    
    float m_scaleLoopSpeed = 2.0f;        
    float m_scaleLoopAmplitude = 0.5f;    
    float m_scaleLoopTimer = 0.0f;        
    float m_baseScaleForLoop = 1.0f;      

    int m_clickCountLimit = 1;        // 何回で消えるか（0なら無限、または指定回数）
    int m_currentClickCount = 0;      // 現在のクリック回数
    bool m_useClickShift = false;     // クリックごとにシートをずらすか
    int m_clickShiftCol = 1;          // クリックごとにずらす列数
    int m_clickShiftRow = 0;          // クリックごとにずらす行数


    HeatTransfer* m_targetHeat = nullptr;
};