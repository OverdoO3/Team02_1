#pragma once

#include <DirectXMath.h>
#include <Effekseer.h>

class Effect
{
public:
	Effect() {};
	~Effect() {};

	Effekseer::Handle Play(const DirectX::XMFLOAT3& position, float scale = 1.0f);

	void Stop(Effekseer::Handle handle);

	void SetPosition(Effekseer::Handle handle, const DirectX::XMFLOAT3& position);

	void SetScale(Effekseer::Handle handle, const DirectX::XMFLOAT3& scale);

	void SetRotation(Effekseer::Handle handle, const DirectX::XMFLOAT3 rotation);

	bool Exists(int handle);

	bool Load(const char* filename);
private:
	Effekseer::EffectRef effekseerEffect;
};