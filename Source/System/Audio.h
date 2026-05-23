#pragma once

#include <xaudio2.h>
#include "System/AudioSource.h"
#include <string>
#include <filesystem>


// オーディオ
class Audio
{
private:
    Audio() = default;
    ~Audio() = default;

public:
    static Audio& Instance()
    {
        static Audio instance;
        return instance;
    }

    // 初期化
    void Initialize();

    // 終了化
    void Finalize();

    // オーディオソース読み込み
    AudioSource* LoadAudioSource(const char* filename);

    // システム状態取得（重要：これがクラッシュを防ぐ）
    static bool IsSystemAlive() { return m_isInitialized; }

    IXAudio2* GetXAudio2() { return xaudio; }

    void PlayBGM(const char* filename, bool force = false);
    void SetBGMVolume(float volume);

    void PlaySE(const char* filename, float volume = 1.0f);
    void UpdateSE();
    void SetLockBGM(bool lock) { m_isBGMLocked = lock; }
    void StopBGM(bool force = false);

    std::string ToDataPath(const std::string& inputPath)
    {
        // 1. まずバックスラッシュ(\)をスラッシュ(/)に統一する（Windowsパス対策）
        std::string path = inputPath;
        std::replace(path.begin(), path.end(), '\\', '/');

        // 2. 検索したいキーワードのリスト
        const std::string keywords[] = { "Data/", "Scenes/" };

        for (const auto& key : keywords)
        {
            size_t pos = path.find(key);
            if (pos != std::string::npos)
            {
                // キーワード以降の文字列を返す
                return path.substr(pos);
            }
        }
        return path;
    }



    const std::string& GetCurrentBGMName() const { return m_currentBGMName; }
private:
    IXAudio2* xaudio = nullptr;
    IXAudio2MasteringVoice* masteringVoice = nullptr;
    static bool             m_isInitialized; // 静的フラグ

    std::unique_ptr<AudioSource> m_bgmSource; 
    std::shared_ptr<AudioResource> m_bgmResource;
    std::string m_currentBGMName = "";
    std::vector<std::unique_ptr<AudioSource>> m_seSources;
    float m_bgmVolume = 1.0f;
    bool m_isBGMLocked = false;
};