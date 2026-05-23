#include "Misc.h"
#include "System/AudioSource.h"
#include "Audio.h"

// コンストラクタ
AudioSource::AudioSource(IXAudio2* xaudio, std::shared_ptr<AudioResource>& resource)
	: resource(resource)
{
	HRESULT hr;

	// ソースボイスを生成
	hr = xaudio->CreateSourceVoice(&sourceVoice, &resource->GetWaveFormat());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

// デストラクタ
AudioSource::~AudioSource()
{

}

// 再生
void AudioSource::Play(bool loop)
{
	Stop();

	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes = resource->GetAudioBytes();
	buffer.pAudioData = resource->GetAudioData();
	buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	sourceVoice->SubmitSourceBuffer(&buffer);

	sourceVoice->SetVolume(m_volume);

	HRESULT hr = sourceVoice->Start();
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}


// 停止
void AudioSource::Stop()
{

	if (sourceVoice != nullptr)
	{
		sourceVoice->FlushSourceBuffers();
		sourceVoice->Stop();
	}
}

// 音量設定
void AudioSource::SetVolume(float volume)
{
	m_volume = volume;
	if (sourceVoice && Audio::IsSystemAlive()) {
		sourceVoice->SetVolume(m_volume);
	}
}

bool AudioSource::IsPlaying()
{
	if (!sourceVoice) return false;

	XAUDIO2_VOICE_STATE state;
	sourceVoice->GetState(&state);

	// バッファがまだ残っているなら再生中とみなす
	return state.BuffersQueued > 0;
}