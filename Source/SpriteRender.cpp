#include "SpriteRender.h"
#include "Factory.h"
#include "Actor.h"
#include "SceneManager.h"
#include "ThermalBody.h"
#include "Actor.h"
#include "System/Audio.h"
#include "firewood.h"

REGISTER_COMPONENT(ComponentID::SpriteRender,SpriteRender)

void SpriteRender::Draw(RenderContext& rc)
{
    if (!owner) return;
    if (m_isClickedHidden) return;
    if (m_isOnlyWhileGoal && !m_isPopUp) return;

    // 異常状態の描画制御
    if (m_showAbnormal)
    {
        bool isNormal = true;
        auto scene = owner->GetScene();
        if (scene)
        {
            for (auto& actor : scene->actors)
            {
                if (actor->tag == 1)
                {
                    auto thermal = actor->GetComponent<ThermalBody>();
                    if (thermal && thermal->GetHeat() != 0) isNormal = false;
                    break;
                }
            }
        }
        if (isNormal) return;
    }

    // 熱UI制御
    if (m_useHeatUI)
    {
        bool showUI = false;
        auto scene = owner->GetScene();

        if (scene)
        {
            for (auto& actor : scene->actors)
            {
                auto receiver = actor->GetComponent<HeatReceiver>();

                // プレイヤーの近くにあるかチェック
                if (receiver && receiver->IsPlayerNear())
                {
                    auto fw = actor->GetComponent<firewood>();

                    if (!fw || fw->GetFire())
                    {
                        showUI = true;
                        break;
                    }
                }
            }
        }

        if (!showUI) return;
    }

    // ポーズUI制御
    if (m_isPauseUI)
    {
        if (!m_sceneManager || (!m_sceneManager->IsPaused() && m_irisMode == IrisMode::None)) return;
    }

    // Loading制御
    if (m_isOnlyWhileLoading)
    {
        if (!m_sceneManager || m_sceneManager->GetLoadState() != SceneManager::LoadState::Loading) return;
    }

    if (m_isGameLoading || m_isTitleLoading)
    {
        std::string nextPath = m_sceneManager ? m_sceneManager->GetPendingScenePath() : "";
        if (!nextPath.empty())
        {
            std::string lowerPath = nextPath;
            std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
            bool hasStage = (lowerPath.find("stage") != std::string::npos);
            bool hasTitle = (lowerPath.find("title") != std::string::npos);
            if (m_isGameLoading && !hasStage) return;
            if (m_isTitleLoading && !hasTitle) return;
        }
        else if (m_irisMode == IrisMode::None)
        {
            return;
        }
    }

    auto tran = owner->GetComponent<Transform>();
    if (spr && tran)
    {
        DirectX::XMFLOAT3 worldPos = tran->GetWorldPosition();
        DirectX::XMFLOAT3 worldScale = tran->GetWorldScale();
        DirectX::XMFLOAT3 worldRot = tran->GetWorldEulerAngles();

        float texW = spr->GetTextureWidth();
        float texH = spr->GetTextureHeight();
        float sw = (m_srcW < 0.0f) ? texW : m_srcW;
        float sh = (m_srcH < 0.0f) ? texH : m_srcH;

        float width = sw * m_editorScale * worldScale.x;
        float height = sh * m_editorScale * worldScale.y;

        ImVec2 screenOffset = ImGui::GetCursorScreenPos();
        float drawX = screenOffset.x + worldPos.x;
        float drawY = screenOffset.y + worldPos.y;

        if (m_pivot == Pivot::Center)
        {
            drawX -= width * 0.5f;
            drawY -= height * 0.5f;
        }

        // ─── ⭕ ゆりかご揺れ（移動＋回転）の反映 ───
        float swingAngleOffset = 0.0f;
        if (m_swingLoop) {
            drawX += sinf(m_cradleTimer * m_swingSpeed) * m_swingAmplitude;
            if (m_rotSwingEnabled) {
                swingAngleOffset = sinf(m_cradleTimer * m_rotSwingSpeed) * m_rotSwingAmplitude;
            }
        }

        m_lastDrawX = drawX;
        m_lastDrawY = drawY;
        m_lastDrawW = width;
        m_lastDrawH = height;

        float totalAngle = m_editorAngleDeg + worldRot.z + swingAngleOffset;

        // 色とアルファの計算
        DirectX::XMFLOAT4 finalColor = color;
        Actor* parent = owner->GetParent();
        if (parent) {
            if (auto* parentSpr = parent->GetComponent<SpriteRender>()) {
                if (parentSpr->m_hoverFade) finalColor.w *= parentSpr->m_appearanceRatio;
            }
        }
        else if (m_hoverFade) {
            finalColor.w *= m_appearanceRatio;
        }
        finalColor.w *= m_fadeAlpha;

        // 親座標系の計算
        parent = owner->GetParent();
        if (parent)
        {
            auto parentTran = parent->GetComponent<Transform>();
            auto* parentSpr = parent->GetComponent<SpriteRender>();
            if (parentTran && parentSpr)
            {
                DirectX::XMFLOAT3 parentPos = parentTran->GetWorldPosition();
                DirectX::XMFLOAT3 parentScale = parentTran->GetWorldScale();

                float parentCenterX = screenOffset.x + parentPos.x;
                float parentCenterY = screenOffset.y + parentPos.y;

                float relX = drawX - parentCenterX;
                float relY = drawY - parentCenterY;

                float rad = DirectX::XMConvertToRadians(totalAngle);
                float cosA = cosf(rad);
                float sinA = sinf(rad);

                drawX = parentCenterX + cosA * relX - sinA * relY;
                drawY = parentCenterY + sinA * relX + cosA * relY;
            }
        }

        // 既存の揺れ処理 (m_swingEnabled)
        if (m_swingEnabled)
        {
            if (m_useSwingX) drawX += (m_swingXUseCos ? cosf((m_swingTimer + m_swingOffsetX) * m_swingSpeedX) : sinf((m_swingTimer + m_swingOffsetX) * m_swingSpeedX)) * m_swingAmplitudeX;
            if (m_useSwingY) drawY += (m_swingYUseCos ? cosf((m_swingTimer + m_swingOffsetY) * m_swingSpeedY) : sinf((m_swingTimer + m_swingOffsetY) * m_swingSpeedY)) * m_swingAmplitudeY;
            if (m_useSwingRot) totalAngle += (m_swingRotUseCos ? cosf(m_swingTimer * m_swingRotSpeed) : sinf(m_swingTimer * m_swingRotSpeed)) * m_swingRotAmplitude;
        }

        // 描画
        spr->Render(rc, drawX, drawY, worldPos.z, width, height, m_srcX, m_srcY, sw, sh, totalAngle, finalColor.x, finalColor.y, finalColor.z, finalColor.w);

#ifdef _DEBUG
        if (m_showCollider)
        {
            float debugLeft = screenOffset.x + worldPos.x + m_colliderOffsetX * worldScale.x;
            float debugTop = screenOffset.y + worldPos.y + m_colliderOffsetY * worldScale.y;
            ImGui::GetForegroundDrawList()->AddRect(ImVec2(debugLeft, debugTop), ImVec2(debugLeft + m_colliderWidth * worldScale.x, debugTop + m_colliderHeight * worldScale.y), m_isHovered ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 255, 0, 255), 0.0f, 0xF, 2.0f);
        }
#endif
    }
}


//void SpriteRender::Update(float elapsedTime)
//{
//    // ── マウスクリック判定 ──────────────────────────────────────
//    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
//    {
//        if (!m_mouseWasDown)
//        {
//            m_mouseWasDown = true;
//
//            POINT mousePos;
//            GetCursorPos(&mousePos);
//            HWND hwnd = GetActiveWindow();
//            ScreenToClient(hwnd, &mousePos);   // スクリーン→クライアント座標
//
//            auto tran = owner->GetComponent<Transform>();
//            if (tran && spr)
//            {
//                auto pos = tran->GetWorldPosition(); // これがスプライト左上のピクセル座標
//
//                float texW = spr->GetTextureWidth();
//                float texH = spr->GetTextureHeight();
//                float sw = (m_srcW < 0.0f) ? texW : m_srcW;
//                float sh = (m_srcH < 0.0f) ? texH : m_srcH;
//                float width = sw * m_editorScale;
//                float height = sh * m_editorScale;
//
//                // Sprite::Render は dx,dy を左上として描画している
//                float left = m_lastDrawX;
//                float top = m_lastDrawY;
//                float right = m_lastDrawX + m_lastDrawW;
//                float bottom = m_lastDrawY + m_lastDrawH;
//
//                if (mousePos.x >= left && mousePos.x <= right &&
//                    mousePos.y >= top && mousePos.y <= bottom)
//                {
//                    color = { 1.0f, 0.0f, 0.0f, 1.0f };
//                }
//            }
//        }
//    }
//    else
//    {
//        m_mouseWasDown = false;
//    }
//
//
//    if (!m_isLoop || m_animFrameCount <= 1) return;
//
//    m_timer += elapsedTime;
//
//    if (m_timer >= m_frameDuration)
//    {
//        m_timer = 0.0f;
//        m_currentFrame++;
//
//        if (m_currentFrame >= m_animFrameCount)
//        {
//            m_currentFrame = 0;
//        }
//
//        if (spr)
//        {
//            float texW = spr->GetTextureWidth();
//            float singleFrameW = texW / (float)m_splitX;
//
//            m_srcW = singleFrameW;
//
//            int nextCol = (m_targetCol + m_currentFrame) % m_splitX;
//            m_srcX = (float)nextCol * m_srcW;
//        }
//    }
//}
void SpriteRender::Update(float elapsedTime)
{
    if (m_isClickedHidden)return;
    if (!owner) return;


    if (m_isPopUp)
    {
        m_popUpTimer += elapsedTime;
        float t = m_popUpTimer / m_popUpDuration;

        if (t >= 1.0f)
        {
            t = 1.0f;

        }

        // イージング計算（シュッと出てバウンドする計算）
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;
        float ease = 1.0f + (c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f));

        // 最大サイズ（m_maxPopScale）で見た目を固定する
        float scaleMultiplier = m_maxPopScale - (ease * (m_maxPopScale - 1.0f));
        m_editorScale = m_baseScale * scaleMultiplier;
    }
    // ─── ⭕ アイリス演出（拡大・縮小）の更新 ───
    if (m_irisMode != IrisMode::None)
    {
        m_irisTimer += elapsedTime;
        float rate = m_irisTimer / m_irisDuration;
        if (rate >= 1.0f) rate = 1.0f;

        if (m_irisMode == IrisMode::Out)
        {
            if (rate >= 0.95f)
            {
                m_editorScale = m_irisTargetScale; // 完全に 0.0f（閉じきった状態）にする
                m_irisMode = IrisMode::None;       // 演出終了

            }
            else
            {
                // 通常通りの爆速縮小イージング
                float easeOutRate = 1.0f - powf(2.0f, -10.0f * rate);
                m_editorScale = m_irisMaxScale + (m_irisTargetScale - m_irisMaxScale) * easeOutRate;
            }
        }
        else if (m_irisMode == IrisMode::In)
        {
            if (rate >= 1.0f)
            {
                m_irisMode = IrisMode::None;
                owner->setActive = false;
                m_editorScale = m_originalScale;
            }
            else
            {
                // ⭕ エルミート補間（Smoothstep）
                // 初速を抑えつつ、後半もダラダラせずにスッと綺麗に開ききります
                float smoothRate = rate * rate * (3.0f - 2.0f * rate);

                m_editorScale = m_irisStartScale + (m_irisMaxScale - m_irisStartScale) * smoothRate;
            }
        }
    }
    if (m_isPauseUI)
    {
        if (!m_sceneManager || !m_sceneManager->IsPaused()) return;
    }

    if (m_swingLoop)
    {
        m_cradleTimer += elapsedTime; 
    }

    // ─── 1. マウスのホバー判定 ───
    auto tran = owner->GetComponent<Transform>();
    if (tran && spr)
    {
        DirectX::XMFLOAT3 worldPos = tran->GetWorldPosition();
        DirectX::XMFLOAT3 worldScale = tran->GetWorldScale();

        float texW = spr->GetTextureWidth();
        float texH = spr->GetTextureHeight();
        float sw = (m_srcW < 0.0f) ? (texW / (float)m_splitX) : m_srcW;
        float sh = (m_srcH < 0.0f) ? (texH / (float)m_splitY) : m_srcH;

        float width = sw * m_editorScale * worldScale.x;
        float height = sh * m_editorScale * worldScale.y;

        float drawLeft = worldPos.x;
        float drawTop = worldPos.y;
        if (m_pivot == Pivot::Center)
        {
            drawLeft -= width * 0.5f;
            drawTop -= height * 0.5f;
        }

        float colLeft = drawLeft + m_colliderOffsetX * worldScale.x;
        float colTop = drawTop + m_colliderOffsetY * worldScale.y;
        float colRight = colLeft + m_colliderWidth * m_editorScale * worldScale.x;
        float colBottom = colTop + m_colliderHeight * m_editorScale * worldScale.y;

        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 contentMin = ImGui::GetWindowContentRegionMin();

        float mouseXInWindow = mousePos.x - (windowPos.x + contentMin.x);
        float mouseYInWindow = mousePos.y - (windowPos.y + contentMin.y);

        m_isHovered = (mouseXInWindow >= colLeft && mouseXInWindow <= colRight &&
            mouseYInWindow >= colTop && mouseYInWindow <= colBottom);

        if (m_isHovered && !m_wasHovered)
        {
            std::string myPath = ToDataPath("Data/Sound/SE_cursor.wav");
            Audio::Instance().PlaySE(myPath.c_str());
        }
        m_wasHovered = m_isHovered;
    }

    // ─── Update 関数内のクリック判定箇所 ───
    if (m_isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        std::string myPath = ToDataPath("Data/Sound/SE_button.wav");
        Audio::Instance().PlaySE(myPath.c_str());

        m_currentClickCount++;

        // 2. スプライトシートをずらす処理
        if (m_useClickShift)
        {
            m_targetCol = (m_targetCol + m_clickShiftCol) % m_splitX;
            m_targetRow = (m_targetRow + m_clickShiftRow) % m_splitY;
        }

        // 3. 回数制限に達したら隠す（制限が0より大きい場合）
        if (m_useClickHide && m_clickCountLimit > 0 && m_currentClickCount >= m_clickCountLimit)
        {
            m_isClickedHidden = true;

            owner->SetChildrenHidden(true);
        }
    }


    // ─── 2. アルファフェード処理 ───
    if (m_hoverFade)
    {
        float targetRatio = m_isHovered ? 1.0f : 0.0f;
        float t = m_fadeSpeed * elapsedTime;
        if (t > 1.0f) t = 1.0f;
        m_appearanceRatio += (targetRatio - m_appearanceRatio) * t;
    }
    else
    {
        m_appearanceRatio = 1.0f;
    }

    for (auto& child : owner->GetChildren())
    {
        if (auto* childSr = child->GetComponent<SpriteRender>())
        {
            // 子のフェード設定（m_hoverFade）も親と同期させたい場合はここでセット
            childSr->m_appearanceRatio = this->m_appearanceRatio;
        }
    }

    // ─── 3. UVアニメーション・テクスチャ切り替え ───
    if (spr)
    {
        int currentCol = m_targetCol;
        int currentRow = m_targetRow;

        if (m_isLoop && m_animFrameCount > 1)
        {
            currentCol = (m_targetCol + m_currentFrame) % m_splitX;
        }

        if (m_hoverSpriteShift && m_isHovered)
        {
            currentCol = (currentCol + m_hoverCollOffset) % m_splitX;
            currentRow = (currentRow + m_hoverRowOffset) % m_splitY;
        }

        float texW = spr->GetTextureWidth();
        float texH = spr->GetTextureHeight();
        float cellW = (m_srcW < 0.0f) ? (texW / (float)m_splitX) : m_srcW;
        float cellH = (m_srcH < 0.0f) ? (texH / (float)m_splitY) : m_srcH;

        m_srcX = (float)currentCol * cellW;
        m_srcY = (float)currentRow * cellH;
    }

    if (m_useHeatUI)
    {
        auto heat = owner->GetComponent<HeatTransfer>();
        if (heat)
        {
            this->enabled = heat->GetCanAbsorb();
        }
    }


    // 揺れ処理
    if (m_swingEnabled)
    {
        m_swingTimer += elapsedTime;
    }

    if (m_isFadeEnabled)
    {
        m_fadeTimer += elapsedTime * m_sprFadeSpeed;
        m_fadeAlpha = (sinf(m_fadeTimer) * 0.5f) + 0.5f;
    }
    else
    {
        m_fadeAlpha = 1.0f;
    }

    if (m_isScaleLoopEnabled)
    {
        m_scaleLoopTimer += elapsedTime * m_scaleLoopSpeed;

        float scaleOffset = sinf(m_scaleLoopTimer) * m_scaleLoopAmplitude;
        m_editorScale = m_baseScaleForLoop + scaleOffset;
    }

    if (!m_isLoop || m_animFrameCount <= 1) return;

    m_timer += elapsedTime;
    if (m_timer >= m_frameDuration)
    {
        m_timer = 0.0f;
        m_currentFrame++;
        if (m_currentFrame >= m_animFrameCount)
            m_currentFrame = 0;
    }

}

std::unique_ptr<Component> SpriteRender::Clone() const
{
	auto c = std::make_unique<SpriteRender>();

    c->texturepath       = this->texturepath;
    c->m_srcX            = this->m_srcX;
    c->m_srcY            = this->m_srcY;
    c->m_srcW            = this->m_srcW;
    c->m_srcH            = this->m_srcH;
    c->m_splitX          = this->m_splitX;
    c->m_splitY          = this->m_splitY;
    c->m_targetCol       = this->m_targetCol;
    c->m_targetRow       = this->m_targetRow;
    c->m_isLoop          = this->m_isLoop;
    c->m_frameDuration   = this->m_frameDuration;
    c->m_animFrameCount  = this->m_animFrameCount;
    c->m_editorScale     = this->m_editorScale;
    c->m_pivot           = this->m_pivot;
    c->sortOrder         = this->sortOrder;
    c->m_editorAngleDeg = this->m_editorAngleDeg;

    c->m_colliderOffsetX = this->m_colliderOffsetX;
    c->m_colliderOffsetY = this->m_colliderOffsetY;
    c->m_colliderWidth   = this->m_colliderWidth;
    c->m_colliderHeight  = this->m_colliderHeight;
    c->m_isPauseUI       = this->m_isPauseUI;


    c->m_hoverFade       = this->m_hoverFade;
    c->m_fadeSpeed       = this->m_fadeSpeed;
    c->m_hoverSpriteShift = this->m_hoverSpriteShift;
    c->m_hoverCollOffset  = this->m_hoverCollOffset;
    c->m_hoverRowOffset = this->m_hoverRowOffset;
    c->m_appearanceRatio  = 0.0;

    c->m_irisDuration = this->m_irisDuration;
    c->m_irisTargetScale = this->m_irisTargetScale;

    if (texturepath != "")
    {
        c->SetSprite(std::make_unique<Sprite>(texturepath.c_str()));
        c->SetString(texturepath.c_str());
    }

    c->m_irisDuration = this->m_irisDuration;
    c->m_irisTargetScale = this->m_irisTargetScale;

    c->m_irisMode = IrisMode::None;
    c->m_irisTimer = 0.0f;
    c->m_originalScale = this->m_editorScale;

    c->m_isGameLoading = this->m_isGameLoading;
    c->m_isTitleLoading = this->m_isTitleLoading;
    c->m_isOnlyWhileLoading = this->m_isOnlyWhileLoading;

    c->m_swingEnabled = this->m_swingEnabled;
    c->m_useSwingX = this->m_useSwingX;
    c->m_useSwingY = this->m_useSwingY;
    c->m_swingXUseCos = this->m_swingXUseCos;
    c->m_swingYUseCos = this->m_swingYUseCos;
    c->m_swingAmplitudeX = this->m_swingAmplitudeX;
    c->m_swingAmplitudeY = this->m_swingAmplitudeY;
    c->m_swingSpeedX = this->m_swingSpeedX;
    c->m_swingSpeedY = this->m_swingSpeedY;
    c->m_swingOffsetX = this->m_swingOffsetX;
    c->m_swingOffsetY = this->m_swingOffsetY;
    c->m_useSwingRot = this->m_useSwingRot;
    c->m_swingRotAmplitude = this->m_swingRotAmplitude;
    c->m_swingRotSpeed = this->m_swingRotSpeed;
    c->m_swingRotUseCos = this->m_swingRotUseCos;
    c->m_isFadeEnabled = this->m_isFadeEnabled;
    c->m_sprFadeSpeed = this->m_sprFadeSpeed;

    c->m_useClickHide = this->m_useClickHide;
    c->m_isClickedHidden = false; // クローンされた側は表示状態からスタート

    c->m_usePopUpClear = this->m_usePopUpClear;
    c->m_popUpDuration = this->m_popUpDuration;
    c->m_maxPopScale = this->m_maxPopScale;
    c->m_isOnlyWhileGoal = this->m_isOnlyWhileGoal;
    c->m_useHeatUI = this->m_useHeatUI;
    c->m_showAbnormal = this->m_showAbnormal;

    c->m_isScaleLoopEnabled = this->m_isScaleLoopEnabled;
    c->m_scaleLoopSpeed = this->m_scaleLoopSpeed;
    c->m_scaleLoopAmplitude = this->m_scaleLoopAmplitude;
    c->m_baseScaleForLoop = this->m_baseScaleForLoop;

    c->m_clickCountLimit = this->m_clickCountLimit;
    c->m_useClickShift = this->m_useClickShift;
    c->m_clickShiftCol = this->m_clickShiftCol;
    c->m_clickShiftRow = this->m_clickShiftRow;

    c->m_swingLoop = this->m_swingLoop;
    c->m_swingAmplitude = this->m_swingAmplitude;
    c->m_swingSpeed = this->m_swingSpeed;
    c->m_rotSwingEnabled = this->m_rotSwingEnabled;
    c->m_rotSwingAmplitude = this->m_rotSwingAmplitude;
    c->m_rotSwingSpeed = this->m_rotSwingSpeed;

    c->m_currentClickCount = 0;

	return c;
}

void SpriteRender::Serialize(nlohmann::json& j) const
{
	j["TexturePath"] = texturepath;
    j["SrcX"] = m_srcX;
    j["SrcY"] = m_srcY;
    j["SrcW"] = m_srcW;
    j["SrcH"] = m_srcH;
    j["ColorR"] = color.x;
    j["ColorG"] = color.y;
    j["ColorB"] = color.z;
    j["ColorA"] = color.w;
    j["SortOrder"] = sortOrder;
    j["SplitX"] = m_splitX;
    j["SplitY"] = m_splitY;
    j["SpriteIndex"] = m_spriteIndex;
    j["TargetCol"] = m_targetCol;
    j["TargetRow"] = m_targetRow;
    j["IsLoop"] = m_isLoop;
    j["FrameDuration"] = m_frameDuration;
    j["AnimFrameCount"] = m_animFrameCount;
    j["Pivot"] = (int)m_pivot;
    j["EditorScale"] = m_editorScale;

    j["ColOffsetX"] = m_colliderOffsetX;
    j["ColOffsetY"] = m_colliderOffsetY;
    j["ColWidth"] = m_colliderWidth;
    j["ColHeight"] = m_colliderHeight;
    j["ShowCollider"] = m_showCollider;
    j["IsPauseUI"] = m_isPauseUI;
    
    j["HoverFade"] = m_hoverFade;
    j["FadeSpeed"] = m_fadeSpeed;

    j["HoverShift"] = m_hoverSpriteShift;
    j["HoverColOffset"] = m_hoverCollOffset;
    j["HoverRowOffset"] = m_hoverRowOffset;

    j["IrisDuration"] = m_irisDuration;
    j["IrisTargetScale"] = m_irisTargetScale;

    j["IsGameLoading"] = m_isGameLoading;
    j["IsTitleLoading"] = m_isTitleLoading;
    j["IsOnlyWhileLoading"] = m_isOnlyWhileLoading;

    j["SwingEnabled"] = m_swingEnabled;
    j["SwingUseX"] = m_useSwingX;
    j["SwingUseY"] = m_useSwingY;
    j["SwingXUseCos"] = m_swingXUseCos;
    j["SwingYUseCos"] = m_swingYUseCos;
    j["SwingAmpX"] = m_swingAmplitudeX;
    j["SwingAmpY"] = m_swingAmplitudeY;
    j["SwingSpeedX"] = m_swingSpeedX;
    j["SwingSpeedY"] = m_swingSpeedY;
    j["SwingOffsetX"] = m_swingOffsetX;
    j["SwingOffsetY"] = m_swingOffsetY;
    j["SwingUseRot"] = m_useSwingRot;
    j["SwingRotAmp"] = m_swingRotAmplitude;
    j["SwingRotSpeed"] = m_swingRotSpeed;
    j["SwingRotUseCos"] = m_swingRotUseCos;
    j["isFadeEnabled"] = m_isFadeEnabled;
    j["sprFadeSpeed"]  = m_sprFadeSpeed;
    j["UseClickHide"] = m_useClickHide;

    j["UsePopUpClear"] = m_usePopUpClear;
    j["PopUpDuration"] = m_popUpDuration;
    j["MaxPopScale"] = m_maxPopScale;
    j["OnlyWhileGoal"] = m_isOnlyWhileGoal;
    j["HeatUI"] = m_useHeatUI;
    j["ShowAbnormal"] = m_showAbnormal;

    j["ScaleLoopEnabled"] = m_isScaleLoopEnabled;
    j["ScaleLoopSpeed"] = m_scaleLoopSpeed;
    j["ScaleLoopAmp"] = m_scaleLoopAmplitude;

    j["ClickLimit"] = m_clickCountLimit;
    j["UseClickShift"] = m_useClickShift;
    j["ClickShiftCol"] = m_clickShiftCol;
    j["ClickShiftRow"] = m_clickShiftRow;

    j["swingLoop"] = m_swingLoop;
    j["swingAmplitude"] = m_swingAmplitude;
    j["swingSpeed"] = m_swingSpeed;
    j["rotSwingEnabled"] = m_rotSwingEnabled;
    j["rotSwingAmplitude"] = m_rotSwingAmplitude;
    j["rotSwingSpeed"] = m_rotSwingSpeed;
}

void SpriteRender::DrawInspector()
{
    ImGui::Checkbox("Enabled", &enabled);
    ImGui::Checkbox("Pause UI Only", &m_isPauseUI);

    ImGui::Text("lastDrawX: %.1f", m_lastDrawX);
    ImGui::Text("lastDrawW: %.1f", m_lastDrawW);

    if (ImGui::Button("Center on Screen X"))
    {
        auto t = owner->GetComponent<Transform>();
        if (t && spr)
        {
            float texW = spr->GetTextureWidth();
            float baseW = (m_srcW < 0.0f)
                ? ((m_splitX > 1) ? (texW / (float)m_splitX) : texW)
                : m_srcW;

            DirectX::XMFLOAT3 localScale = t->GetLocalScale();
            float uiWidth = baseW * m_editorScale * localScale.x;

            float targetX = (m_pivot == Pivot::Center)
                ? 640.0f
                : 640.0f - uiWidth * 0.5f;

            DirectX::XMFLOAT3 localPos = t->GetLocalPosition();
            localPos.x = targetX;
            t->SetLocalPosition(localPos);
        }
    }

    if (ImGui::CollapsingHeader("Hover Fade Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Use Hover Fade", &m_hoverFade);
        if (m_hoverFade)
        {
            ImGui::DragFloat("Fade Speed", &m_fadeSpeed, 0.1f, 0.1f, 30.0f, "%.1f");
        }
    }
    ImGui::Separator();

    if(ImGui::CollapsingHeader("Hover Shift Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Use Hover Shift", &m_hoverSpriteShift);
        if (m_hoverSpriteShift)
        {
            ImGui::InputInt("Shift Column Offset (X)", &m_hoverCollOffset);
            ImGui::InputInt("Shift Row Offset (Y)", &m_hoverRowOffset); 
        }
    }
    ImGui::Separator();


    auto tran = owner->GetComponent<Transform>();
    if (!tran) return;

    if (ImGui::CollapsingHeader("Basic Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        DirectX::XMFLOAT3 pos = tran->GetLocalPosition();
        if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
            tran->SetLocalPosition(pos);
        }

        float rotZ = tran->GetLocalEulerAngles().z;
        if (ImGui::DragFloat("UI Rotation Z", &rotZ, 1.0f)) {
            // SetRotationEuler は Radians を期待しているはずなので変換してセット
            tran->SetRotationEuler(
                tran->GetEulerRotation().x, // X, Y は今の値を維持
                tran->GetEulerRotation().y,
                DirectX::XMConvertToRadians(rotZ)
            );
        }

        float scaleVal = tran->GetLocalScale().x; 
        if (ImGui::DragFloat("UI Scale", &scaleVal, 0.01f, 0.001f, 1000.0f)) {
            tran->SetLocalScale({ scaleVal, scaleVal, 1.0f });
        }


        ImGui::Separator();
        ImGui::ColorEdit4("Color", &color.x);
        ImGui::InputInt("Sort Order", &sortOrder);
    }

    ImGui::Separator(); // 境界線
    ImGui::Text("Fade Settings");

    // ⭕ フェード有効化のチェックボックス
    ImGui::Checkbox("Enable Fade Loop", &m_isFadeEnabled);

    if (m_isFadeEnabled)
    {
        ImGui::SliderFloat("Fade Speed", &m_sprFadeSpeed, 0.1f, 10.0f, "%.1f");

        ImGui::Text("Current Alpha: %.2f", color.w);
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Click Actions", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Use Click Hide", &m_useClickHide);
        if (m_useClickHide)
        {
            ImGui::InputInt("Click Limit Count", &m_clickCountLimit);
            ImGui::Text("Current Clicks: %d", m_currentClickCount);
            if (ImGui::Button("Reset Click Count")) m_currentClickCount = 0;
        }

        ImGui::Checkbox("Use Click Shift", &m_useClickShift);
        if (m_useClickShift)
        {
            ImGui::InputInt("Shift Col per Click", &m_clickShiftCol);
            ImGui::InputInt("Shift Row per Click", &m_clickShiftRow);
        }

        if (m_isClickedHidden && ImGui::Button("Reset Visibility"))
        {
            m_isClickedHidden = false;
            m_currentClickCount = 0;
        }
    }
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Sprite Resource"))
    {
        if (!texturepath.empty()) {
            std::string filename = std::filesystem::path(texturepath).filename().string();
            ImGui::Text("File: %s", filename.c_str());
        }

        if (ImGui::Button("Select Sprite")) {
            std::string full = OpenDialog::OpenLoadFileDialog();
            if (!full.empty()) {
                texturepath = ToDataPath(full);
                spr = std::make_unique<Sprite>(texturepath.c_str());
            }
        }
    }

    ImGui::Separator();
    ImGui::Text("Goal PopUp Setting");

    ImGui::Checkbox("Use Goal PopUp", &m_usePopUpClear);
    ImGui::Checkbox("Only While Goal", &m_isOnlyWhileGoal);

    if (m_usePopUpClear)
    {
        ImGui::SliderFloat("Pop Duration", &m_popUpDuration, 0.1, 2.0f);
        ImGui::SliderFloat("Max Pop Scale", &m_maxPopScale, 1.1f, 3.0f);
    }
    ImGui::Separator();


    if (ImGui::CollapsingHeader("Sprite Sheet Splitter"))
    {
        ImGui::InputInt("Columns (Horizontal)", &m_splitX);
        ImGui::InputInt("Rows (Vertical)", &m_splitY);
        ImGui::InputInt("Select Column (X)", &m_targetCol);
        ImGui::InputInt("Select Row (Y)", &m_targetRow);
        ImGui::InputInt("Sprite Index", &m_spriteIndex);

        if (m_splitX < 1) m_splitX = 1;
        if (m_splitY < 1) m_splitY = 1; 
        if (m_spriteIndex < 0) m_spriteIndex = 0;

        m_targetCol = std::clamp(m_targetCol, 0, m_splitX - 1);
        m_targetRow = std::clamp(m_targetRow, 0, m_splitY - 1);

        if (spr && ImGui::Button("Update Crop Area")) {
            float texW = spr->GetTextureWidth();
            float texH = spr->GetTextureHeight();
            m_srcW = texW / (float)m_splitX;
            m_srcH = texH / (float)m_splitY;
            m_srcX = (float)m_targetCol * m_srcW;
            m_srcY = (float)m_targetRow * m_srcH;
        }
    }

    ImGui::Separator();
    ImGui::Text("Scale Loop Settings");

    ImGui::Checkbox("Enable Scale Loop", &m_isScaleLoopEnabled);
    if (m_isScaleLoopEnabled)
    {
        ImGui::SliderFloat("Loop Speed", &m_scaleLoopSpeed, 0.1f, 10.0f, "%.1f");
        ImGui::SliderFloat("Loop Amplitude", &m_scaleLoopAmplitude, 0.01f, 2.0f, "%.2f");

        // 基準スケールをリセットするボタンがあると便利
        if (ImGui::Button("Reset Base Scale"))
        {
            m_baseScaleForLoop = m_editorScale;
        }
    }

    if (ImGui::CollapsingHeader("UV Crop"))
    {
        ImGui::DragFloat("Src X", &m_srcX, 1.0f, 0.0f, 4096.0f);
        ImGui::DragFloat("Src Y", &m_srcY, 1.0f, 0.0f, 4096.0f);
        ImGui::DragFloat("Src W", &m_srcW, 1.0f, -1.0f, 4096.0f);
        ImGui::DragFloat("Src H", &m_srcH, 1.0f, -1.0f, 4096.0f);
        ImGui::Text("(-1 = full texture)");
    }
    if (ImGui::CollapsingHeader("Cradle Swing Settings"))
    {
        ImGui::Checkbox("Swing Loop", &m_swingLoop);
        if (m_swingLoop)
        {
            ImGui::SliderFloat("Swing Amplitude", &m_swingAmplitude, 0.0f, 100.0f);
            ImGui::SliderFloat("Swing Speed", &m_swingSpeed, 0.1f, 10.0f);

            ImGui::Separator();
            ImGui::Checkbox("Rotate Swing", &m_rotSwingEnabled);
            if (m_rotSwingEnabled)
            {
                ImGui::SliderFloat("Rot Amplitude", &m_rotSwingAmplitude, 0.0f, 1.0f);
                ImGui::SliderFloat("Rot Speed", &m_rotSwingSpeed, 0.1f, 10.0f);
            }
        }
    }
    if (ImGui::CollapsingHeader("Animation Settings"))
    {
        ImGui::Checkbox("Loop Animation", &m_isLoop);
        ImGui::DragInt("Frame Count", &m_animFrameCount, 1, 1, m_splitX);
        ImGui::SliderFloat("Frame Duration", &m_frameDuration, 0.01f, 2.0f, "%.2f sec");

        if (ImGui::Button("Reset Animation")) {
            m_currentFrame = 0;
            m_timer = 0.0f;
        }
    }

    if (ImGui::CollapsingHeader("Pivot"))
    {
        const char* pivotItems[] = { "TopLeft", "Center" };
        int pivotIndex = (int)m_pivot;
        ImGui::PushID("PivotCombo");
        if (ImGui::Combo("Pivot", &pivotIndex, pivotItems, 2))
        {
            m_pivot = (Pivot)pivotIndex;
        }
        ImGui::PopID();
    }
    ImGui::Separator();

    ImGui::Checkbox("Show Abnormal", &m_showAbnormal);
    ImGui::Checkbox("Show Heat UI", &m_useHeatUI);
    if (m_useHeatUI)
    {
        ImGui::Text("HeatTransfer component is active.");
    }
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Swing Settings"))
    {
        ImGui::Checkbox("Enable Swing", &m_swingEnabled);
        if (m_swingEnabled)
        {
            ImGui::Separator();
            ImGui::Text("--- X Swing ---");
            ImGui::Checkbox("Use X Swing", &m_useSwingX);
            if (m_useSwingX)
            {
                ImGui::Checkbox("X Use Cos (off=Sin)", &m_swingXUseCos);
                ImGui::DragFloat("X Amplitude", &m_swingAmplitudeX, 0.5f, -500.0f, 500.0f);
                ImGui::DragFloat("X Speed", &m_swingSpeedX, 0.1f, 0.0f, 20.0f);
                ImGui::DragFloat("X Phase Offset", &m_swingOffsetX, 0.1f, -10.0f, 10.0f);
            }

            ImGui::Separator();
            ImGui::Text("--- Y Swing ---");
            ImGui::Checkbox("Use Y Swing", &m_useSwingY);
            if (m_useSwingY)
            {
                ImGui::Checkbox("Y Use Cos (off=Sin)", &m_swingYUseCos);
                ImGui::DragFloat("Y Amplitude", &m_swingAmplitudeY, 0.5f, -500.0f, 500.0f);
                ImGui::DragFloat("Y Speed", &m_swingSpeedY, 0.1f, 0.0f, 20.0f);
                ImGui::DragFloat("Y Phase Offset", &m_swingOffsetY, 0.1f, -10.0f, 10.0f);
            }

            ImGui::Separator();
            ImGui::Text("--- Rotation Swing ---");
            ImGui::Checkbox("Use Rotation Swing", &m_useSwingRot);
            if (m_useSwingRot)
            {
                ImGui::Checkbox("Rot Use Cos (off=Sin)", &m_swingRotUseCos);
                ImGui::DragFloat("Rot Amplitude (deg)", &m_swingRotAmplitude, 0.5f, -180.0f, 180.0f);
                ImGui::DragFloat("Rot Speed", &m_swingRotSpeed, 0.1f, 0.0f, 20.0f);
            }

            if (ImGui::Button("Reset Timer"))
                m_swingTimer = 0.0f;
        }
    }

    ImGui::Separator();
    ImGui::Text("--- Iris Out Setting ---");

    // インスペクターから秒数と目標サイズをスライダー等で調整できるようにする
    ImGui::DragFloat("Iris Duration (sec)", &m_irisDuration, 0.1f, 0.0f, 10.0f, "%.2f");
    ImGui::DragFloat("Iris Target Scale", &m_irisTargetScale, 0.01f, 0.0f, 5.0f, "%.2f");

    ImGui::Separator();
    ImGui::Text("--- Loading Sprite Settings ---");
    ImGui::Checkbox("Use on Game Loading", &m_isGameLoading);
    ImGui::Checkbox("Use on Title Loading", &m_isTitleLoading);
    ImGui::Checkbox("Only While Loading", &m_isOnlyWhileLoading);
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Box Collider 2D"))
    {
        //表示・非表示のスイッチ
        ImGui::Checkbox("Show Collider OutLine", &m_showCollider);

        ImGui::DragFloat("Offset X", &m_colliderOffsetX, 1.0f);
        ImGui::DragFloat("Offset Y", &m_colliderOffsetY, 1.0f);
        ImGui::DragFloat("Width", &m_colliderWidth, 1.0f, 1.0f, 2048.0f);
        ImGui::DragFloat("Height", &m_colliderHeight, 1.0f, 1.0f, 2048.0f);

        if (ImGui::Button("Fit to Sprite")) {
            // ボタン一発でスプライトのサイズに合わせる機能
            m_colliderWidth = m_lastDrawW;
            m_colliderHeight = m_lastDrawH;
            m_colliderOffsetX = (m_pivot == Pivot::Center) ? -(m_lastDrawW * 0.5f) : 0.0f;
            m_colliderOffsetY = (m_pivot == Pivot::Center) ? -(m_lastDrawH * 0.5f) : 0.0f;
        }
    }
}


void SpriteRender::Deserialize(nlohmann::json& j)
{
	texturepath = j["TexturePath"];
    m_srcX = j.value("SrcX", 0.0f);
    m_srcY = j.value("SrcY", 0.0f);
    m_srcW = j.value("SrcW", -1.0f);
    m_srcH = j.value("SrcH", -1.0f);
    color.x = j.value("ColorR", 1.0f);
    color.y = j.value("ColorG", 1.0f);
    color.z = j.value("ColorB", 1.0f);
    color.w = j.value("ColorA", 1.0f);
    sortOrder = j.value("SortOrder", 0);
    m_splitX = j.value("SplitX", 1);
    m_splitY = j.value("SplitY", 1);
    m_targetCol = j.value("TargetCol", 0);
    m_targetRow = j.value("TargetRow", 0);

	if (texturepath != "")
	{
		spr = std::make_unique<Sprite>(texturepath.c_str());

        if (spr)
        {
            float texW = spr->GetTextureWidth();
            float texH = spr->GetTextureHeight();
            // ⭕ 幅・高さがデフォルト値（-1.0f）の時だけ、テクスチャ全体 or 分割サイズを入れる
            if (m_srcW < 0.0f) m_srcW = texW / (float)m_splitX;
            if (m_srcH < 0.0f) m_srcH = texH / (float)m_splitY;

            // ⭕【重要】JSONに "SrcX" や "SrcY" が保存されていなかった場合（または0の初期状態）だけ自動計算する
            // これで、手動で弄った UVCrop の値が上書きされるのを防ぎます！
            if (!j.contains("SrcX")) m_srcX = (float)m_targetCol * m_srcW;
            if (!j.contains("SrcY")) m_srcY = (float)m_targetRow * m_srcH;
        }
    }
	

    m_spriteIndex = j.value("SpriteIndex", 0);

    m_isLoop = j.value("IsLoop", false);
    m_frameDuration = j.value("FrameDuration", 0.1f);
    m_animFrameCount = j.value("AnimFrameCount", 1);
    m_pivot = (Pivot)j.value("Pivot", 0);

    m_colliderOffsetX = j.value("ColOffsetX", 0.0f);
    m_colliderOffsetY = j.value("ColOffsetY", 0.0f);
    m_colliderWidth = j.value("ColWidth", 100.0f);
    m_colliderHeight = j.value("ColHeight", 100.0f);
    m_showCollider = j.value("ShowCollider", true);
    m_isPauseUI = j.value("IsPauseUI", false);

    m_hoverFade = j.value("HoverFade", false);
    m_fadeSpeed = j.value("FadeSpeed", 5.0f);
    m_appearanceRatio = 0.0f; 

    m_hoverSpriteShift = j.value("HoverShift", false);
    m_hoverCollOffset = j.value("HoverColOffset", 1);
    m_hoverRowOffset = j.value("HoverRowOffset", 0);

    m_irisDuration = j.value("IrisDuration",m_irisDuration);
    m_irisTargetScale = j.value("IrisTargetScale", m_originalScale);

    m_isGameLoading = j.value("IsGameLoading", false);
    m_isTitleLoading = j.value("IsTitleLoading", false);
    m_isOnlyWhileLoading = j.value("IsOnlyWhileLoading", false);

    m_swingEnabled = j.value("SwingEnabled", false);
    m_useSwingX = j.value("SwingUseX", false);
    m_useSwingY = j.value("SwingUseY", false);
    m_swingXUseCos = j.value("SwingXUseCos", false);
    m_swingYUseCos = j.value("SwingYUseCos", true);
    m_swingAmplitudeX = j.value("SwingAmpX", 10.0f);
    m_swingAmplitudeY = j.value("SwingAmpY", 10.0f);
    m_swingSpeedX = j.value("SwingSpeedX", 1.0f);
    m_swingSpeedY = j.value("SwingSpeedY", 1.0f);
    m_swingOffsetX = j.value("SwingOffsetX", 0.0f);
    m_swingOffsetY = j.value("SwingOffsetY", 0.0f);
    m_useSwingRot = j.value("SwingUseRot", false);
    m_swingRotAmplitude = j.value("SwingRotAmp", 5.0f);
    m_swingRotSpeed = j.value("SwingRotSpeed", 1.0f);
    m_swingRotUseCos = j.value("SwingRotUseCos", false);
    m_isFadeEnabled = j.value("isFadeEnabled",false);
    m_sprFadeSpeed = j.value("sprFadeSpeed", 5.0f);
    m_useClickHide = j.value("UseClickHide", false);
    m_isClickedHidden = false; // 読み込み時は一頭表示状態に戻す

    m_usePopUpClear = j.value("UsePopUpClear", false);
    m_popUpDuration = j.value("PopUpDuration", 1.0f);
    m_maxPopScale   = j.value("MaxPopScale", 1.0f);
    m_isOnlyWhileGoal = j.value("OnlyWhileGoal", false);
    m_useHeatUI = j.value("HeatUI", false);
    m_showAbnormal = j.value("ShowAbnormal", false);

    m_clickCountLimit = j.value("ClickLimit", 1);
    m_useClickShift = j.value("UseClickShift", false);
    m_clickShiftCol = j.value("ClickShiftCol", 1);
    m_clickShiftRow = j.value("ClickShiftRow", 0);
    m_currentClickCount = 0; // 読み込み時はリセット

    m_isScaleLoopEnabled = j.value("ScaleLoopEnabled", false);
    m_scaleLoopSpeed = j.value("ScaleLoopSpeed", 1.0f);
    m_scaleLoopAmplitude = j.value("ScaleLoopAmp", 0.1f);
    m_baseScaleForLoop = m_editorScale;

    m_swingLoop = j.value("swingLoop",false);
    m_swingAmplitude = j.value("swingAmplitude",0.0f);
    m_swingSpeed = j.value("swingSpeed",0.0f);

    m_rotSwingEnabled = j.value("rotSwingEnabled",false);
    m_rotSwingAmplitude = j.value("rotSwingAmplitude",0.0f);
    m_rotSwingSpeed = j.value("rotSwingSpeed",0.0f);
}

void SpriteRender::StartIrisOut()
{

    m_irisMode = IrisMode::Out;
    m_irisTimer = 0.0f;

    m_irisDuration = 0.6f;     // 2回目も絶対に爆速（0.25秒）
    m_irisTargetScale = 0.10f;   // 完全に閉じきる

    m_editorScale = m_originalScale; // 確実に全開サイズからスタートさせる
    m_irisMaxScale = m_editorScale;
}

void SpriteRender::StartIrisIn()
{

    m_irisMode = IrisMode::In;
    m_irisTimer = 0.0f;

    m_irisDuration = 0.7f;      // 2回目も絶対に爆速（0.3秒）
    m_irisMaxScale = 6.0f;    // 遥か彼方まで拡大する超特大スケール

    m_irisStartScale = m_editorScale; // 現在のサイズ（0.0fなど）からスタート

}

void SpriteRender::StartPopUp(float duration, float maxScale)
{
    m_isPopUp = true;
    m_popUpTimer = 0.0f;
    m_popUpDuration = duration;
    m_maxPopScale = maxScale;

    m_baseScale = m_editorScale;
}