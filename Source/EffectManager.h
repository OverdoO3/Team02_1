#pragma once
#include <DirectXMath.h>
#include <Effekseer.h>
#include <EffekseerRendererDX11.h>
#include <unordered_map>
#include <memory>
#include "Effect.h"

class EffectManager
{
private:
	EffectManager() {};
	~EffectManager() {};
public:
	static EffectManager& Instance()
	{
		static EffectManager e;
		return e;
	}

	void Initialize();

	void Finalize();

	void Update(float elapsedTime);

	void Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

	std::shared_ptr<Effect> LoadEffect(const std::string& path);

	Effekseer::ManagerRef GetEffekseerManager() { return effekseerManager; }
private:
	Effekseer::ManagerRef          effekseerManager;
	EffekseerRenderer::RendererRef effekseerRenderer;

	std::unordered_map<std::string, std::shared_ptr<Effect>> effectCache;
};