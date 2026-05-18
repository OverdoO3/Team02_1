#include "SpriteRender.h"
#include "Factory.h"
#include "Actor.h"
#include "SceneManager.h"

REGISTER_COMPONENT(ComponentID::SpriteRender,SpriteRender)

void SpriteRender::Draw(RenderContext& rc)
{
    if (!owner)return;

    //ポーズUI制御
    if (m_isPauseUI)
    {
        // ブレークポイントを置く
        bool isNull = (m_sceneManager == nullptr);
        bool isPaused = m_sceneManager ? m_sceneManager->IsPaused() : false;
        if (!m_sceneManager || !m_sceneManager->IsPaused()) return;
    }

    auto tran = owner->GetComponent<Transform>();
    if (spr && tran)
    {
        DirectX::XMFLOAT3 worldPos = tran->GetWorldPosition();
        DirectX::XMFLOAT3 worldScale = tran->GetWorldScale();
        DirectX::XMFLOAT3 worldRot = tran->GetWorldEulerAngles(); // オイラー角(Degree)を想定

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

        // キャッシュ（当たり判定用）
        m_lastDrawX = drawX;
        m_lastDrawY = drawY;
        m_lastDrawW = width;
        m_lastDrawH = height;

        float totalAngle = m_editorAngleDeg + worldRot.z;

        //描画に使う色を決定する(デバッグ用)
        DirectX::XMFLOAT4 finalColor = color;

#ifdef _DEBUG
        // もしデバッグ中かつコライダー表示がONで、ホバー中なら描画色を赤にする
        if (m_showCollider && m_isHovered)
        {
            finalColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // デバッグ用：赤
        }
#endif // _DEBUG


        // 実際の描画呼び出し
        spr->Render(
            rc,
            drawX, drawY, worldPos.z,
            width, height,
            m_srcX, m_srcY,
            sw, sh,
            totalAngle,
            finalColor.x, finalColor.y, finalColor.z, finalColor.w 
        );
#ifdef _DEBUG
        if (m_showCollider)
        {
            // 当たり判定の枠もスケールさせる
            float debugLeft = screenOffset.x + worldPos.x + m_colliderOffsetX * worldScale.x;
            float debugTop = screenOffset.y + worldPos.y + m_colliderOffsetY * worldScale.y;
            float debugW = m_colliderWidth * worldScale.x;
            float debugH = m_colliderHeight * worldScale.y;

            ImU32 colliderColor = m_isHovered
                ? IM_COL32(255, 0, 0, 255)   // マウスが乗ったら赤
                : IM_COL32(0, 255, 0, 255);  // 通常時は緑

            ImGui::GetForegroundDrawList()->AddRect(
                ImVec2(debugLeft, debugTop),
                ImVec2(debugLeft + debugW, debugTop + debugH),
                colliderColor, 
                0.0f, 0xF, 2.0f
            );
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
    if (!owner)return;
    // 1. マウス座標とウィンドウの開始位置を取得

    if (m_isPauseUI)
    {
        if (!m_sceneManager || !m_sceneManager->IsPaused()) return;
    }

    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 screenOffset = ImGui::GetCursorScreenPos();

    // 2. Transformから現在の世界座標を取得
    auto tran = owner->GetComponent<Transform>();
    if (tran)
    {
        DirectX::XMFLOAT3 worldPos = tran->GetWorldPosition();

        // 当たり判定の矩形範囲を計算 (スクリーン絶対座標系)
        float colLeft = screenOffset.x + worldPos.x + m_colliderOffsetX;
        float colTop = screenOffset.y + worldPos.y + m_colliderOffsetY;
        float colRight = colLeft + m_colliderWidth;
        float colBottom = colTop + m_colliderHeight;

        m_isHovered = (mousePos.x >= colLeft && mousePos.x <= colRight &&
            mousePos.y >= colTop && mousePos.y <= colBottom);

    }

    //アニメーション処理
    if (!m_isLoop || m_animFrameCount <= 1) return;

    m_timer += elapsedTime;
    if (m_timer >= m_frameDuration)
    {
        m_timer = 0.0f;
        m_currentFrame++;
        if (m_currentFrame >= m_animFrameCount)
            m_currentFrame = 0;

        if (spr)
        {
            float texW = spr->GetTextureWidth();
            float singleFrameW = texW / (float)m_splitX;
            m_srcW = singleFrameW;
            int nextCol = (m_targetCol + m_currentFrame) % m_splitX;
            m_srcX = (float)nextCol * m_srcW;
        }
    }
}




std::unique_ptr<Component> SpriteRender::Clone() const
{
	auto c = std::make_unique<SpriteRender>();

    c->texturepath = this->texturepath;
    c->m_srcX = this->m_srcX;
    c->m_srcY = this->m_srcY;
    c->m_srcW = this->m_srcW;
    c->m_srcH = this->m_srcH;
    c->m_splitX = this->m_splitX;
    c->m_splitY = this->m_splitY;
    c->m_targetCol = this->m_targetCol;
    c->m_targetRow = this->m_targetRow;
    c->m_isLoop = this->m_isLoop;
    c->m_frameDuration = this->m_frameDuration;
    c->m_animFrameCount = this->m_animFrameCount;
    c->m_editorScale = this->m_editorScale;
    c->m_pivot = this->m_pivot;
    c->sortOrder = this->sortOrder;

    c->m_colliderOffsetX = this->m_colliderOffsetX;
    c->m_colliderOffsetY = this->m_colliderOffsetY;
    c->m_colliderWidth = this->m_colliderWidth;
    c->m_colliderHeight = this->m_colliderHeight;

    c->m_isPauseUI = this->m_isPauseUI;

    if (texturepath != "")
    {
        c->SetSprite(std::make_unique<Sprite>(texturepath.c_str()));
        c->SetString(texturepath.c_str());
    }

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
}

void SpriteRender::DrawInspector()
{
    ImGui::Checkbox("Enabled", &enabled);
    ImGui::Checkbox("Pause UI Only", &m_isPauseUI);

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
        if (ImGui::DragFloat("UI Scale", &scaleVal, 0.01f, 0.001f, 100.0f)) {
            tran->SetLocalScale({ scaleVal, scaleVal, 1.0f });
        }


        ImGui::Separator();
        ImGui::ColorEdit4("Color", &color.x);
        ImGui::InputInt("Sort Order", &sortOrder);
    }

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

    if (ImGui::CollapsingHeader("UV Crop"))
    {
        ImGui::DragFloat("Src X", &m_srcX, 1.0f, 0.0f, 4096.0f);
        ImGui::DragFloat("Src Y", &m_srcY, 1.0f, 0.0f, 4096.0f);
        ImGui::DragFloat("Src W", &m_srcW, 1.0f, -1.0f, 4096.0f);
        ImGui::DragFloat("Src H", &m_srcH, 1.0f, -1.0f, 4096.0f);
        ImGui::Text("(-1 = full texture)");
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
        if (ImGui::Combo("Pivot", &pivotIndex, pivotItems, 2))
        {
            m_pivot = (Pivot)pivotIndex;
        }
    }

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


    m_spriteIndex = j.value("SpriteIndex", 0);

	if (texturepath != "")
	{
		spr = std::make_unique<Sprite>(texturepath.c_str());
	}
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
}