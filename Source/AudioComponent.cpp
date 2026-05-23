#include "AudioComponent.h"
#include "Factory.h"
#include <imgui.h>
#include <direct.h>
#include "OpenDialog.h"
#include <filesystem>

REGISTER_COMPONENT(ComponentID::Audio, AudioComponent)

void AudioComponent::OnAwake(float elapsedTime)
{
    if (source)
        source->SetVolume(m_volume);

    if (m_playOnAwake && source)
        source->Play(m_loop);
}


void AudioComponent::DrawInspector()
{
    if (ImGui::CollapsingHeader("Audio Component"))
    {
        char buf[256];
        strncpy_s(buf, sizeof(buf), m_filename.c_str(), _TRUNCATE);

        if (ImGui::InputText("Audio File", buf, sizeof(buf)))
        {
            m_filename = buf;
        }

        if (ImGui::Button("Select Audio File"))
        {
            std::string full = OpenDialog::OpenLoadFileDialog();
            if (!full.empty())
            {
                std::filesystem::path base = std::filesystem::absolute("Data");
                std::filesystem::path target = std::filesystem::absolute(full);
                std::filesystem::path relative = std::filesystem::relative(target, base);
                m_filename = "Data/" + relative.lexically_normal().generic_string();
                Load(m_filename.c_str());
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Play")) { Play(); 
        }
        ImGui::Separator();
        ImGui::Checkbox("Play On Awake", &m_playOnAwake);
        ImGui::Checkbox("Loop", &m_loop);
        ImGui::SliderFloat("Volume", &m_volume, 0.0f, 1.0f);
        if (ImGui::Button("Play")) Play(m_loop);
        if (ImGui::Button("Stop")) Stop();
    }
}

void AudioComponent::Serialize(json& j) const
{
    j["filename"] = m_filename;
    j["PlayOnAwake"] = m_playOnAwake;
    j["Loop"] = m_loop;
    j["Volume"] = m_volume;
}

void AudioComponent::Deserialize(json& j)
{
    m_filename = j.value("filename", "");
    if (!m_filename.empty())
    {
        Load(m_filename.c_str());
    }
    m_playOnAwake = j.value("PlayOnAwake", false);
    m_loop = j.value("Loop", false);
    m_volume = j.value("Volume", 1.0f);

    if (source) source->SetVolume(m_volume);
}

std::unique_ptr<Component> AudioComponent::Clone() const
{
    auto clone = std::make_unique<AudioComponent>();
    clone->m_filename = this->m_filename;
    if (!clone->m_filename.empty())
    {
        clone->Load(clone->m_filename.c_str());
    }
    clone->m_playOnAwake = this->m_playOnAwake;
    clone->m_loop = this->m_loop;
    clone->m_volume = this->m_volume;

    return clone;
}

void AudioComponent::Load(const char* filename)
{
    if (filename == nullptr || strlen(filename) == 0) return;

    std::string filenameStr(filename);  // ‚Ü‚¸string‚ÉƒRƒs[
    if (filenameStr.empty()) return;

    if (source)
    {
        delete source;
        source = nullptr;
    }

    source = Audio::Instance().LoadAudioSource(filenameStr.c_str());
}

void AudioComponent::Play(bool loop) { if (source) source->Play(loop); }
void AudioComponent::Stop() { if (source) source->Stop(); }
void AudioComponent::SetVolume(float volume) { if (source) source->SetVolume(volume); }