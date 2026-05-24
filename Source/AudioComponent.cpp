#include "AudioComponent.h"
#include "Factory.h"
#include <imgui.h>
#include "OpenDialog.h"
#include <filesystem>
#include "Actor.h"
#include "SpriteRender.h"
#include "Input.h"

REGISTER_COMPONENT(ComponentID::Audio, AudioComponent)

void AudioComponent::OnAwake(float elapsedTime)
{
    if (!Audio::IsSystemAlive())return;
    if (m_isBGM) {
        if (m_bgmPlayOnAwake) {
            SetVolume(m_volume);
            Audio::Instance().PlayBGM(m_filename.c_str());
        }
    }
    else {
        if (source) {
            source->SetVolume(m_volume);
            if (m_playOnAwake) {
                source->Play(m_loop);
            }
        }
    }
}

void AudioComponent::Update(float elapsedTime)
{
    if (!Audio::IsSystemAlive()) return;
    if (!owner) return;

    auto sprite = owner->GetComponent<SpriteRender>();
    bool isHovered = (sprite != nullptr && sprite->IsHovered());

    // 1. ホバー音の再生処理
    if (m_playOnHover && isHovered) {
        // まだ再生中でない場合だけ Play を呼ぶ
        if (!m_isPlaying) {
            Play(m_loop);
        }
    }
    else if (m_playOnHover && !isHovered) {
        // ホバーが外れたら「再生終了」とみなしてフラグをオフにする
        m_isPlaying = false;
    }

    // 2. クリック音の再生処理
    if (m_playOnClick && isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        Play(m_loop);
    }
}

void AudioComponent::DrawInspector()
{
    if (!Audio::IsSystemAlive()) return;
    if (!owner) return;

    if (ImGui::CollapsingHeader("Audio Component"))
    {
        ImGui::Checkbox("Is BGM", &m_isBGM);

        // 2. モード別設定
        if (m_isBGM) {
            ImGui::Checkbox("BGM Play On Awake", &m_bgmPlayOnAwake);

            // 操作ボタン
            if (ImGui::Button("Play BGM")) {
                Audio::Instance().PlayBGM(m_filename.c_str());
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop BGM")) {
                Audio::Instance().StopBGM();
            }
        }
        else {
            ImGui::Checkbox("Play On Awake", &m_playOnAwake);
            ImGui::Checkbox("Loop", &m_loop);
            ImGui::Checkbox("Play On Click", &m_playOnClick);
            ImGui::Checkbox("Play On Hover", &m_playOnHover);
        }

        ImGui::Separator();

        // 3. ファイル操作
        if (!m_filename.empty()) {
            ImGui::Text("File: %s", m_filename.c_str());
        }
        else {
            ImGui::Text("File: [None]");
        }

        char buf[256] = { 0 };
        strncpy_s(buf, m_filename.c_str(), _TRUNCATE);
        if (ImGui::InputText("Audio File", buf, sizeof(buf))) {
            m_filename = buf;
        }

        if (ImGui::Button("Select File")) {
            std::string full = OpenDialog::OpenLoadFileDialog();
            if (!full.empty()) {
                m_filename = ToDataPath(full);
                Load(m_filename.c_str());
            }
        }

        ImGui::Separator();

        // 4. 音量
        if (ImGui::SliderFloat("Volume", &m_volume, 0.0f, 1.0f)) {
            SetVolume(m_volume);
        }
    }
}

std::unique_ptr<Component> AudioComponent::Clone() const
{
    auto clone = std::make_unique<AudioComponent>();
    clone->m_filename = this->m_filename;
    clone->m_playOnAwake = this->m_playOnAwake;
    clone->m_bgmPlayOnAwake = this->m_bgmPlayOnAwake;
    clone->m_loop = this->m_loop;
    clone->m_volume = this->m_volume;
    clone->m_playOnClick = this->m_playOnClick;
    clone->m_playOnHover = this->m_playOnHover;
    clone->m_isBGM = this->m_isBGM;

    if (!clone->m_filename.empty()) {
        clone->Load(clone->m_filename.c_str());
    }
    return clone;
}

void AudioComponent::Serialize(nlohmann::json& j) const
{
    j["filename"] = m_filename;
    j["PlayOnAwake"] = m_playOnAwake;
    j["BGMPlayOnAwake"] = m_bgmPlayOnAwake;
    j["Loop"] = m_loop;
    j["Volume"] = m_volume;
    j["PlayOnClick"] = m_playOnClick;
    j["PlayOnHover"] = m_playOnHover;
    j["IsBGM"] = m_isBGM;
}

void AudioComponent::Deserialize(nlohmann::json& j)
{
    m_filename = j.value("filename", "");
    if (!m_filename.empty()) Load(m_filename.c_str());

    m_playOnAwake = j.value("PlayOnAwake", false);
    m_bgmPlayOnAwake = j.value("BGMPlayOnAwake", false);
    m_loop = j.value("Loop", false);
    m_volume = j.value("Volume", 1.0f);
    m_playOnClick = j.value("PlayOnClick", false);
    m_playOnHover = j.value("PlayOnHover", false);
    m_isBGM = j.value("IsBGM", false);

    if (source) source->SetVolume(m_volume);
}

void AudioComponent::Load(const char* filename)
{
    if (!Audio::IsSystemAlive() || m_isBGM) return;
    if (!filename || strlen(filename) == 0) return;

    // ★ここで変換する
    std::string safePath = ToDataPath(filename);

    auto res = std::make_shared<AudioResource>(safePath.c_str());
    source = std::make_unique<AudioSource>(Audio::Instance().GetXAudio2(), res);
}

void AudioComponent::Play(bool loop)
{
    if (!Audio::IsSystemAlive()) return;
    if (source) {
        source->Play(loop);
        m_isPlaying = true; // 再生開始！
    }
}
void AudioComponent::Stop() { if (source) source->Stop(); }
void AudioComponent::SetVolume(float volume)
{
    m_volume = volume;

    if (m_isBGM) {
        // Audioクラス側へ音量を飛ばす
        Audio::Instance().SetBGMVolume(m_volume);
    }
    else {
        // 通常のSEは自分自身の source を更新
        if (source) source->SetVolume(m_volume);
    }
}