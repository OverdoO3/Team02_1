#pragma once
#include <Windows.h>
#include "System/Sprite.h"
#include <memory>

class MouseCursor
{
public:
	MouseCursor() : m_textureID(0), m_isPressed(false)
	{
		m_pos.x = 0;m_pos.y = 0;
	}
	~MouseCursor();

	void Initialize(const char* filepath);
	void Update(HWND hWnd);
	void Draw(RenderContext& rc);

private:
	std::unique_ptr<Sprite> spr;

	int m_textureID;	//テクスチャの番号
	POINT m_pos;		//現在の座標
	bool m_isPressed;	//クリック中かを判定
};