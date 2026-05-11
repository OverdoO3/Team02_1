#include "EffectManager.h"
#include "System/Graphics.h"

void EffectManager::Initialize()
{
	Graphics& graphics = Graphics::Instance();

	effekseerRenderer = EffekseerRendererDX11::Renderer::Create(graphics.GetDevice(), graphics.GetDeviceContext(),2048);

	effekseerManager = Effekseer::Manager::Create(2048);
	//Effekseerレンダラの各種設定
	effekseerManager->SetSpriteRenderer(effekseerRenderer->CreateSpriteRenderer());
	effekseerManager->SetRibbonRenderer(effekseerRenderer->CreateRibbonRenderer());
	effekseerManager->SetRingRenderer(effekseerRenderer->CreateRingRenderer());
	effekseerManager->SetTrackRenderer(effekseerRenderer->CreateTrackRenderer());
	effekseerManager->SetModelRenderer(effekseerRenderer->CreateModelRenderer());
	//Effekseer内でのローダー設定
	effekseerManager->SetTextureLoader(effekseerRenderer->CreateTextureLoader());
	effekseerManager->SetModelLoader(effekseerRenderer->CreateModelLoader());
	effekseerManager->SetMaterialLoader(effekseerRenderer->CreateMaterialLoader());

	//ここで左手座標系で計算する
	effekseerManager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

void EffectManager::Finalize()
{
	//スマートポインタやから大丈夫
}

void EffectManager::Update(float elapsedTime)
{
	//エフェクト更新処理
	effekseerManager->Update(elapsedTime * 60.0f);
}

void EffectManager::Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	effekseerRenderer->SetCameraMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&view));
	effekseerRenderer->SetProjectionMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&projection));

	//Effekseer描画開始
	effekseerRenderer->BeginRendering();

	//Effekseer描画実行
	//マネージャー単位で描画するので描画順を制御するときはマネージャーを複数個作成し、
	//Draw()関数を事項する順序で制御できる
	effekseerManager->Draw();

	//Effekseer描画終了
	effekseerRenderer->EndRendering();
}

std::shared_ptr<Effect> EffectManager::LoadEffect(const std::string& path)
{
	auto it = effectCache.find(path);
	if (it != effectCache.end())
		return it->second;

	auto effect = std::make_shared<Effect>();

	if (!effect->Load(path.c_str()))
		return nullptr;

	effectCache[path] = effect;
	return effect;
}
