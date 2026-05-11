#pragma once
#include <WinUser.h>

class InputC
{
public:
    static bool KeyDown(int key)
    {
        return GetAsyncKeyState(key) & 0x8000;
    }

    static bool KeyPressed(int key)
    {
        static SHORT prev[256] = {};

        SHORT now = GetAsyncKeyState(key);

        bool pressed = (now & 0x8000) && !(prev[key] & 0x8000);

        prev[key] = now;

        return pressed;
    }

    static bool MouseDown(int button)
    {
        return GetAsyncKeyState(button) & 0x8000;
    }
};