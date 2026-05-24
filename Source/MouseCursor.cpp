#include "MouseCursor.h"
#include "imgui.h"
#include "SceneManager.h"

void MouseCursor::Initialize(const char* filepath)
{
	spr = std::make_unique<Sprite>(filepath);
	
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
    // SceneManagerが正しく設定されているか確認
    if (!m_sceneManager) return;

    while (ShowCursor(FALSE) >= 0);

    // 現在のシーンパスを取得して "stage" が含まれるか判定
    std::string path = m_sceneManager->GetCurrentScenePath();
    bool isStageScene = (path.find("stage") != std::string::npos);

    // ゲームがアクティブか（ポーズ中・ロード中でない）判定
    bool isGameActive = !isPaused && !isLoading;

    bool isCursorHidden = isStageScene && isGameActive;

    // カスタムカーソルを表示する条件
    bool shouldShowCursor = isStageScene && isGameActive;

    // 画面外判定
    POINT globalPos;
    GetCursorPos(&globalPos);
    POINT clientPos = globalPos;
    ScreenToClient(hWnd, &clientPos);

    RECT rect;
    GetClientRect(hWnd, &rect);
    bool isOutside = (clientPos.x < 0 || clientPos.x > rect.right ||
        clientPos.y < 0 || clientPos.y > rect.bottom);

    // カーソルの表示制御
    // 画面外、またはゲームがアクティブでない（ポーズ・ロード中等）場合はOSカーソルを出す
    if (isOutside || !shouldShowCursor) {
        while (ShowCursor(TRUE) < 0); // OSカーソルを表示
        m_showCustomCursor = false;    // カスタムカーソルを非表示
    }
    else {
        // ゲーム中かつ画面内ならカスタムカーソルを表示
        while (ShowCursor(FALSE) >= 0); // OSカーソルを非表示
        m_showCustomCursor = true;     // カスタムカーソルを表示
    }

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
    // OSカーソルを忘れずに復活させる
    while (ShowCursor(TRUE) < 0);
}