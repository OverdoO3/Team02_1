#pragma once

#include <Windows.h>
#include <map>

class KeyInput
{
private:
    struct KeyBank
    {
        int keyCode;
        bool upKey = false;
        bool downKey = false;
        bool holdKey = false;
    };

    std::map<int, KeyBank> keyBanks;

    //KeyBank* findKeyBank(int keyCode)
    //{
    //    for (auto& key : keyBanks)
    //    {
    //        if (key.keyCode == keyCode)
    //            return &key;
    //    }
    //    return nullptr;
    //}

private:

    KeyInput() {}
    ~KeyInput() {}

public:

    static KeyInput& Instance()
    {
        static KeyInput instans;
        return instans;
    }

    void Update()
    {
        for (auto& [code, key] : keyBanks)
        {
            if (GetAsyncKeyState(key.keyCode) & 0x8000)
            {
                if (key.holdKey == false)
                {
                    key.downKey = true;
                }
                else
                {
                    key.downKey = false;
                }
                key.upKey = false;
                key.holdKey = true;
            }
            else
            {
                if (key.holdKey == true)
                {
                    key.upKey = true;
                }
                else
                {
                    key.upKey = false;
                }
                key.downKey = false;
                key.holdKey = false;
            }
        }
    }

public:
    void SetKey(int keyCode)
    {
        keyBanks.try_emplace(keyCode, KeyBank{keyCode});
    }


    bool GetKeyDown(int keyCode)
    {
        auto it = keyBanks.find(keyCode);
        if (it != keyBanks.end())
            return it->second.downKey;
        else
        {
            SetKey(keyCode);
            return GetKeyDown(keyCode);
        }
    }

    bool GetKeyUp(int keyCode)
    {
        auto it = keyBanks.find(keyCode);
        if (it != keyBanks.end())
            return it->second.upKey;
        else
        {
            SetKey(keyCode);
            return GetKeyUp(keyCode);
        }
    }

    bool GetKeyHold(int keyCode)
    {
        auto it = keyBanks.find(keyCode);
        if (it != keyBanks.end())
            return it->second.holdKey;
        else
        {
            SetKey(keyCode);
            return GetKeyHold(keyCode);
        }
    }
};