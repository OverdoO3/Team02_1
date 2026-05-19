#include "Engine.h"

void Engine::Initialize()
{
	sceneManager.Initialize();
#ifdef _DEBUG
	int sw = GetSystemMetrics(SM_CXSCREEN);
	int sh = GetSystemMetrics(SM_CYSCREEN);

	Graphics::Instance().CreateRenderTarget(sceneRT, sw, sh);
	Graphics::Instance().CreateRenderTarget(gameRT, sw, sh);
#else
	int sw = GetSystemMetrics(SM_CXSCREEN);
	int sh = GetSystemMetrics(SM_CYSCREEN);
	Graphics::Instance().CreateRenderTarget(gameRT, sw, sh); // ★ モニター解像度
#endif
}

void Engine::Update(float dt)
{
	sceneManager.Update(dt);
}

void Engine::Render(CameraBase* editCam, CameraBase* gameCam)
{
	auto* scene = sceneManager.GetCurrentScene();
	if (!scene) return;
	// SceneView用
	SetRenderTarget(sceneRT);
	Clear(sceneRT);
	sceneManager.Render(editCam, true);

	// Game用
	SetRenderTarget(gameRT);
	Clear(gameRT);
	sceneManager.Render(gameCam, false);
}

void Engine::RenderScene(EditorCamera* camera)
{
	auto* scene = sceneManager.GetCurrentScene();
	if (!scene) return;

	auto* dc = Graphics::Instance().GetDeviceContext();

	// =====================
	// Scene描画
	// =====================
	dc->OMSetRenderTargets(1, sceneRT.rtv.GetAddressOf(), sceneRT.dsv.Get());

	D3D11_VIEWPORT vp{};
	vp.Width = (float)sceneRT.width;
	vp.Height = (float)sceneRT.height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	dc->RSSetViewports(1, &vp);

	float clear[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	dc->ClearRenderTargetView(sceneRT.rtv.Get(), clear);
	dc->ClearDepthStencilView(sceneRT.dsv.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f, 0);

	scene->Render(camera, true);
}


void Engine::RenderGame(EditorCamera* camera)
{
	auto* scene = sceneManager.GetCurrentScene();
	if (!scene) return;

	auto* dc = Graphics::Instance().GetDeviceContext();
	// =====================
	// Game描画
	// =====================
	dc->OMSetRenderTargets(1, gameRT.rtv.GetAddressOf(), gameRT.dsv.Get());

	D3D11_VIEWPORT vp{};
	vp.Width = (float)gameRT.width;
	vp.Height = (float)gameRT.height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	dc->RSSetViewports(1, &vp);

	float clear[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	dc->ClearRenderTargetView(gameRT.rtv.Get(), clear);
	dc->ClearDepthStencilView(gameRT.dsv.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f, 0);

	sceneManager.Render(
		scene->nowCamera ? scene->nowCamera->GetComponent<Camera>() : nullptr,
		false
	);
}


void Engine::SetRenderTarget(RenderTarget& rt)
{
	auto& graphics = Graphics::Instance();
	auto* dc = graphics.GetDeviceContext();

	dc->OMSetRenderTargets(1, rt.rtv.GetAddressOf(), rt.dsv.Get());

	D3D11_VIEWPORT vp{};
	vp.Width = (float)rt.width;
	vp.Height = (float)rt.height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	dc->RSSetViewports(1, &vp);
}

void Engine::Clear(RenderTarget& rt)
{
	auto& graphics = Graphics::Instance();
	auto* dc = graphics.GetDeviceContext();

	float clear[4] = {0.1f, 0.1f, 0.1f, 1.0f };

	dc->ClearRenderTargetView(rt.rtv.Get(), clear);
	dc->ClearDepthStencilView(rt.dsv.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f, 0);
}