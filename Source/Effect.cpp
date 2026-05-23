#include "System/Graphics.h"
#include "Effect.h"
#include "EffectManager.h"

Effekseer::Handle Effect::Play(const DirectX::XMFLOAT3& position, float scale)
{
	Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();

	Effekseer::Handle handle = effekseerManager->Play(effekseerEffect, position.x, position.y, position.z);
	effekseerManager->SetScale(handle, scale, scale, scale);
	return handle;
}

void Effect::Stop(Effekseer::Handle handle)
{
	Effekseer::ManagerRef effekseerManger = EffectManager::Instance().GetEffekseerManager();

	effekseerManger->StopEffect(handle);
}

void Effect::SetPosition(Effekseer::Handle handle, const DirectX::XMFLOAT3& position)
{
	Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();

	effekseerManager->SetLocation(handle, position.x, position.y, position.z);
}

void Effect::SetScale(Effekseer::Handle handle, const DirectX::XMFLOAT3& scale)
{
	Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();

	effekseerManager->SetScale(handle, scale.x, scale.y, scale.z);
}

void Effect::SetRotation(Effekseer::Handle handle, const DirectX::XMFLOAT3 rotation)
{
	Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();

	effekseerManager->SetRotation(handle, rotation.x, rotation.y, rotation.z);
}

bool Effect::Exists(int handle)
{
	return EffectManager::Instance().GetEffekseerManager()->Exists(handle);
}

bool Effect::Load(const char* filename)
{
	Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();
	if (!filename) return false;

	// UTF8 → UTF16 変換
	char16_t utf16Filename[256] = {};
	Effekseer::ConvertUtf8ToUtf16(utf16Filename, 256, filename);

	// Effekseerエフェクト作成
	effekseerEffect = Effekseer::Effect::Create(
		effekseerManager,
		(EFK_CHAR*)utf16Filename);

	return effekseerEffect != nullptr;
}


