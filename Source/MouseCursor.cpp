#include "MouseCursor.h"

void MouseCursor::Initialize(const char* filepath)
{
	spr = std::make_unique<Sprite>(filepath);
	
	//画像をロード
	//OSのカーソルを非表示
	while (ShowCursor(FALSE) >= 0);
}

void MouseCursor::Update(HWND hWnd)
{
	GetCursorPos(&m_pos);
	ScreenToClient(hWnd, &m_pos);

	//クリック判定
	m_isPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

}

void MouseCursor::Draw(RenderContext& rc)
{
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