#pragma once

#include <windows.h>
#include "System/HighResolutionTimer.h"


#include "Engine.h"
#include "Editor.h"
#include "MouseCursor.h"

class Framework
{
public:
	Framework(HWND hWnd);
	~Framework();

private:
	void Update(float elapsedTime);
	void Render(float elapsedTime);

	void CalculateFrameStats();

	std::string ToDataPath(const std::string& fullPath)
	{
		std::filesystem::path base = std::filesystem::absolute("Data");
		std::filesystem::path target = std::filesystem::absolute(fullPath);

		std::filesystem::path relative = std::filesystem::relative(target, base);

		std::filesystem::path normalized = relative.lexically_normal();

		return "Data/" + normalized.generic_string();
	}

public:
	int Run();
	LRESULT CALLBACK HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	const HWND				hWnd;
	HighResolutionTimer		timer;

	Engine engine;
	Editor editor;

	//マウスカーソル用
	std::unique_ptr<MouseCursor> mouseCursor;
	//警告防止用
	std::unique_ptr<MouseGuard> mouseGuard;
};

