#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"
#include "System/Audio.h" // 既存のAudioシステム

using json = nlohmann::json;

class AudioComponent : public Component
{
public:
    AudioComponent() = default;
    ~AudioComponent() override {
        if (!Audio::IsSystemAlive()) return;
        Stop();
        
    }

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;

    // 音再生用インターフェース
    void Play(bool loop = false);
    void Stop();
    void SetVolume(float volume);

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
    COMPONENT_ID(Audio)

private:
    void Load(const char* filename);

    std::unique_ptr<AudioSource>source;

    // 複数のファイルを保持できるリスト
    std::vector<std::string> m_filenames;
    // 複数のソースを保持できるリスト
    std::vector<std::unique_ptr<AudioSource>> m_sources;
    // AudioSource* source = nullptr;
    std::string m_filename = "";

    bool m_playOnAwake = false;
    bool m_loop = false;
    float m_volume = 1.0f;

    bool m_isLoop = false;        
    bool m_playOnClick = false;   
    bool m_playOnHover = false;   
    bool m_isBGM = false;
    bool m_bgmPlayOnAwake = true;
    bool m_isPlaying = false;
};    
