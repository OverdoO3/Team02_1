#include "System/Misc.h"
#include "System/Audio.h"
#include <filesystem>

// 静的メンバの初期化
bool Audio::m_isInitialized = false;

// 初期化
void Audio::Initialize()
{
    HRESULT hr;

    // COMの初期化
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    UINT32 createFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    //createFlags |= XAUDIO2_DEBUG_ENGINE;
#endif

    // XAudio初期化
    hr = XAudio2Create(&xaudio, createFlags);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    // マスタリングボイス生成
    hr = xaudio->CreateMasteringVoice(&masteringVoice);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    // システム稼働フラグを立てる
    m_isInitialized = true;
}

// 終了化
void Audio::Finalize()
{
    // 最初にフラグを折ることで、以降のUpdateやDrawInspectorからのアクセスを拒否する
    m_isInitialized = false;

    // マスタリングボイス破棄
    if (masteringVoice != nullptr)
    {
        masteringVoice->DestroyVoice();
        masteringVoice = nullptr;
    }

    // XAudio終了化
    if (xaudio != nullptr)
    {
        xaudio->Release();
        xaudio = nullptr;
    }

    // COM終了化
    CoUninitialize();
}

// オーディオソース読み込み
AudioSource* Audio::LoadAudioSource(const char* filename)
{
    // システムが死んでいる時に読み込もうとしたら警告を出す
    if (!m_isInitialized) return nullptr;

    std::shared_ptr<AudioResource> resource = std::make_shared<AudioResource>(filename);
    return new AudioSource(xaudio, resource);
}

// Audio.cpp

void Audio::PlayBGM(const char* filename, bool force)
{
    // ロック中なら無視
    if (m_isBGMLocked && !force) return;

    // 今流れている曲と同じなら何もしない
    if (m_bgmSource && m_currentBGMName == filename) return;

    StopBGM(true);

    // 新しい曲をロードして再生
    m_bgmResource = std::make_shared<AudioResource>(filename);
    m_bgmSource = std::make_unique<AudioSource>(xaudio, m_bgmResource);
    m_bgmSource->SetVolume(m_bgmVolume);
    m_bgmSource->Play(true);
    m_currentBGMName = filename;
}


void Audio::StopBGM(bool force)
{

    if (!m_isInitialized) return;

    if (m_bgmSource) {
        m_bgmSource->Stop();
        m_bgmSource.reset(); // ボイスを解放
        m_currentBGMName = "";
    }
}
void Audio::SetBGMVolume(float volume)
{
    m_bgmVolume = volume; // 音量を保存
    if (m_bgmSource) {
        m_bgmSource->SetVolume(m_bgmVolume); // 鳴っているボイスに適用
    }
}

void Audio::PlaySE(const char* filename, float volume)
{
    if (!m_isInitialized || xaudio == nullptr) return;

    if (!std::filesystem::exists(filename)) {
        // パスが間違っているとここで引っかかるので、デバッグ出力して止める
        OutputDebugStringA(("!!! File not found: " + std::string(filename) + "\n").c_str());
        return;
    }

    auto res = std::make_shared<AudioResource>(filename);
    auto source = std::make_unique<AudioSource>(xaudio, res);
    source->SetVolume(volume);
    source->Play(false);
    m_seSources.push_back(std::move(source));
}


void Audio::UpdateSE()
{
    // 終わったSEを削除（これでメモリ溢れを防ぐ）
    m_seSources.erase(std::remove_if(m_seSources.begin(), m_seSources.end(),
        [](const auto& s) {
            // ※ここでさっき追加した IsPlaying() を使う！
            return !s->IsPlaying();
        }), m_seSources.end());
}