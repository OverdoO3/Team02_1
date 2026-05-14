#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <mutex>
#include "RenderState.h"
#include "ShapeRenderer.h"
#include "ModelRenderer.h"
#include <DirectXTex.h>

struct RenderTarget
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;

	int width = 0;
	int height = 0;
};

// グラフィックス
class Graphics
{
private:
	Graphics() = default;
	~Graphics() = default;

public:
	// インスタンス取得
	static Graphics& Instance()
	{
		static Graphics instance;
		return instance;
	}

	// 初期化
	void Initialize(HWND hWnd);

	// クリア
	void Clear(float r, float g, float b, float a);

	// レンダーターゲット設定
	void SetRenderTargets();

	// 画面表示
	void Present(UINT syncInterval);

	//影用バッファのクリア
	void ClearShadowMap();

	//影描画の開始
	void BeginShadowMap();

	// ウインドウハンドル取得
	HWND GetWindowHandle() { return hWnd; }

	// デバイス取得
	ID3D11Device* GetDevice() { return device.Get(); }

	// デバイスコンテキスト取得
	ID3D11DeviceContext* GetDeviceContext() { return immediateContext.Get(); }

	// スクリーン幅取得
	float GetScreenWidth() const { return screenWidth; }

	// スクリーン高さ取得
	float GetScreenHeight() const { return screenHeight; }

	// レンダーステート取得
	RenderState* GetRenderState() { return renderState.get(); }

	// シェイプレンダラ取得
	ShapeRenderer* GetShapeRenderer() const { return shapeRenderer.get(); }

	// モデルレンダラ取得
	ModelRenderer* GetModelRenderer() const { return modelRenderer.get(); }

	// 現在のレンダーターゲットビュー（バックバッファ）を取得
	ID3D11RenderTargetView* GetRenderTargetView() const { return renderTargetView.Get(); }

	// バックバッファのポインタを直接返す（OMSetRenderTargets用）
	ID3D11RenderTargetView** GetRenderTargetViewAddressOf() { return renderTargetView.GetAddressOf(); }

	std::recursive_mutex& GetMutex() { return mutex; }
	
	void EndShadowMap();

	bool CreateRenderTarget(RenderTarget& rt, int w, int h);

	ID3D11RenderTargetView* GetBackBufferRTV() const { return renderTargetView.Get(); }
	ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView.Get(); }
	D3D11_VIEWPORT GetViewport() const { return viewport; }

	ID3D11ShaderResourceView* GetShadowMapSRV() const { return shadowMapSRV.Get(); }
	ID3D11SamplerState* GetShadowSampler() const { return shadowSampler.Get(); }

	struct scene_constants
	{
		DirectX::XMFLOAT4X4 view_projection;
		DirectX::XMFLOAT4 options;	//	xy : マウスの座標値, z : タイマー, w : フラグ
		DirectX::XMFLOAT4 camera_position;
	};


	Microsoft::WRL::ComPtr<ID3D11Buffer> scene_constant_buffer;


private:
	HWND											hWnd = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Device>			device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		immediateContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain>			swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	depthStencilView;
	D3D11_VIEWPORT									viewport;

	float	screenWidth = 0;
	float	screenHeight = 0;

	std::unique_ptr<RenderState>					renderState;
	std::unique_ptr<ShapeRenderer>					shapeRenderer;
	std::unique_ptr<ModelRenderer>					modelRenderer;

	std::recursive_mutex mutex;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowMapSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	 shadowMapDSV;
	D3D11_VIEWPORT									 shadowViewport;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>			 shadowTexture;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>		 shadowSampler;
	//元のレンダーターゲットを一時的に保存するための変数
	UINT viewport_count{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
	D3D11_VIEWPORT cached_viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cached_render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cached_depth_stencil_view;
};
