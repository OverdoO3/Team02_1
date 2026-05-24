#pragma once
#include <Windows.h>
#include "System/Sprite.h"
#include <memory>

class SceneManager;

class MouseGuard
{
public:
	//生成されたらマウスを隠す
	MouseGuard() {
		//while (ShowCursor(FALSE) >= 0);
	}

	//破棄されたら見れるようにする
	~MouseGuard() {
		while (ShowCursor(TRUE) < 0);
	}

	// コピーされるとカウンタがおかしくなるので禁止
	MouseGuard(const MouseGuard&) = delete;
	MouseGuard& operator=(const MouseGuard&) = delete;
};

class MouseCursor
{
public:
	MouseCursor(SceneManager* sm) : m_sceneManager(sm), m_textureID(0), m_isPressed(false)
	{
		m_pos.x = 0; m_pos.y = 0;
	}
	~MouseCursor();

	void Initialize(const char* filepath);
	void Update(HWND hWnd, bool isPaused, bool isLoading);
	void Draw(RenderContext& rc);

private:
	std::unique_ptr<Sprite> spr;
	SceneManager* m_sceneManager;
	int m_textureID;	//テクスチャの番号
	POINT m_pos;		//現在の座標
	bool m_isPressed;	//クリック中かを判定
	bool m_showCustomCursor = true;
};