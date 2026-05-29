#include "MouseCursor.h"
#include "imgui.h"
#include "SceneManager.h"

void MouseCursor::Initialize(const char* filepath)
{
	spr = std::make_unique<Sprite>(filepath);
	
#ifdef _DEBUG
    // デバッグ時はOSカーソルを表示しておく
    while (ShowCursor(TRUE) < 0);
#else
    // リリース時は隠す
    while (ShowCursor(FALSE) >= 0);
#endif
	//画像をロード
	//OSのカーソルを非表示
    CURSORINFO ci = { sizeof(CURSORINFO) };
    if (GetCursorInfo(&ci)) {
        if (ci.flags & CURSOR_SHOWING) {
            ShowCursor(FALSE);
        }
    }
}

void MouseCursor::Update(HWND hWnd, bool isPaused, bool isLoading)
{
    if (!m_sceneManager) return;
#ifdef _DEBUG
    // デバッグ時は常にOSカーソルを表示
    while (ShowCursor(TRUE) < 0);
#else
    // リリース時は常にOSカーソルを隠す
    while (ShowCursor(FALSE) >= 0);
#endif


    // 2. 現在の状況を取得
    std::string path = m_sceneManager->GetCurrentScenePath();
    bool isStageScene = (path.find("stage") != std::string::npos);
    bool isGameActive = !isPaused && !isLoading;

    // 3. 以下の条件で「カーソルを隠すべき（＝ゲームプレイ中）」と判定
    bool isGameRunning = (isStageScene && isGameActive);

    // 4. 画面外判定
    POINT globalPos;
    GetCursorPos(&globalPos);
    POINT clientPos = globalPos;
    ScreenToClient(hWnd, &clientPos);

    RECT rect;
    GetClientRect(hWnd, &rect);
    bool isOutside = (clientPos.x < 0 || clientPos.x > rect.right ||
        clientPos.y < 0 || clientPos.y > rect.bottom);

    // 5. 自作カーソルの描画フラグ制御
    // 「ゲームプレイ中」または「画面外」なら自作カーソルも消す
#ifdef _DEBUG
    m_showCustomCursor = false; // デバッグ時は自作カーソルは非表示
#else
    isGameRunning = isStageScene && !isPaused;

    if (isGameRunning && !isOutside) {
        m_showCustomCursor = false; // ゲーム中は自作カーソルを消す
    }
    else {
        m_showCustomCursor = true;  // それ以外（メニュー画面やポーズ中）は出す
    }
#endif
    m_pos = clientPos;
    m_isPressed = ImGui::IsMouseDown(0);
}


void MouseCursor::Draw(RenderContext& rc)
{
    if (!m_showCustomCursor || !spr) return;
    if (!rc.deviceContext) return;

    if (spr)
    {
        float cursorDisplaySize = 32.0f;        
        float offset = cursorDisplaySize * 0.5f;

        float sx = m_isPressed ? 300.0f : 0.0f;
        float sw = 300.0f;
        float sh = 300.0f;

        float dx = (float)m_pos.x - offset;
        float dy = (float)m_pos.y - offset;

        spr->Render(
            rc,
            dx, dy,
            0.0f,
            cursorDisplaySize, cursorDisplaySize, 
            sx, 0.0f,
            sw, sh,
            0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        );
    }
}

// メモリリーク対策：デストラクタでしっかり片付ける
MouseCursor::~MouseCursor()
{
#ifndef _DEBUG
    // ── リリースモードの時だけOSカーソルを復活させる ──
    while (ShowCursor(TRUE) < 0);
#endif
}