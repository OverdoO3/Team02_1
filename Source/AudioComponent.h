#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"
#include "System/Audio.h" // 既存のAudioシステム

using json = nlohmann::json;

class AudioComponent : public Component
{
public:
    AudioComponent() = default;
    ~AudioComponent() override { if (source) delete source; }

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override {}

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;

    // 音再生用インターフェース
    void Play(bool loop = false);
    void Stop();
    void SetVolume(float volume);

    COMPONENT_ID(Audio)

private:
    void Load(const char* filename);

    AudioSource* source = nullptr;
    std::string m_filename = "";

    bool m_playOnAwake = false;
    bool m_loop = false;
    float m_volume = 1.0f;
};