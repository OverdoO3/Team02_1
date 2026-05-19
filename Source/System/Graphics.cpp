#include "Misc.h"
#include "System/Graphics.h"

// 初期化
void Graphics::Initialize(HWND hWnd)
{
	this->hWnd = hWnd;
	// 画面のサイズを取得する。
	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT screenWidth = rc.right - rc.left;
	UINT screenHeight = rc.bottom - rc.top;

	this->screenWidth = static_cast<float>(screenWidth);
	this->screenHeight = static_cast<float>(screenHeight);

	HRESULT hr = S_OK;

	

	// デバイス＆スワップチェーンの生成
	{
		UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
		};

		// スワップチェーンを作成するための設定オプション
		DXGI_SWAP_CHAIN_DESC swapchainDesc;
		{
			swapchainDesc.BufferDesc.Width = screenWidth;
			swapchainDesc.BufferDesc.Height = screenHeight;
			swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
			swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
			swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			swapchainDesc.SampleDesc.Count = 1;
			swapchainDesc.SampleDesc.Quality = 0;
			swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchainDesc.BufferCount = 2;
			swapchainDesc.OutputWindow = hWnd;
			swapchainDesc.Windowed = TRUE;
			swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapchainDesc.Flags = 0;
		}

		D3D_FEATURE_LEVEL featureLevel;

		// デバイス＆スワップチェーンの生成
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createDeviceFlags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&swapchainDesc,
			swapchain.GetAddressOf(),
			device.GetAddressOf(),
			&featureLevel,
			immediateContext.GetAddressOf()
		);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// レンダーターゲットビューの生成
	{
		// スワップチェーンからバックバッファテクスチャを取得する。
		// ※スワップチェーンに内包されているバックバッファテクスチャは'色'を書き込むテクスチャ。
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
		hr = swapchain->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			reinterpret_cast<void**>(texture2d.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		// バックバッファテクスチャへの書き込みの窓口となるレンダーターゲットビューを生成する。
		hr = device->CreateRenderTargetView(texture2d.Get(), nullptr, renderTargetView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// 深度ステンシルビューの生成
	{
		// 深度ステンシル情報を書き込むためのテクスチャを作成する。
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
		D3D11_TEXTURE2D_DESC texture2dDesc;
		texture2dDesc.Width = screenWidth;
		texture2dDesc.Height = screenHeight;
		texture2dDesc.MipLevels = 1;
		texture2dDesc.ArraySize = 1;
		texture2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		texture2dDesc.SampleDesc.Count = 1;
		texture2dDesc.SampleDesc.Quality = 0;
		texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
		texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texture2dDesc.CPUAccessFlags = 0;
		texture2dDesc.MiscFlags = 0;
		hr = device->CreateTexture2D(&texture2dDesc, nullptr, texture2d.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		// 深度ステンシルテクスチャへの書き込みに窓口になる深度ステンシルビューを作成する。
		hr = device->CreateDepthStencilView(texture2d.Get(), nullptr, depthStencilView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// ビューポート
	{
		viewport.Width = static_cast<float>(screenWidth);
		viewport.Height = static_cast<float>(screenHeight);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
	}

	//シャドウマップのリソース生成
	{
		//影用の奥行きを記録するテクスチャを作る
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = 2048;
		texDesc.Height = 2048;
		//texDesc.Width = 2048;
		//texDesc.Height = 2048;

		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		hr = device->CreateTexture2D(&texDesc, nullptr, shadowTexture.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		//影を書き込む窓口
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		hr = device->CreateDepthStencilView(shadowTexture.Get(), &dsvDesc, shadowMapDSV.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		//影をシェーダーで読み込む
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		hr = device->CreateShaderResourceView(shadowTexture.Get(), &srvDesc, shadowMapSRV.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		shadowViewport.Width = 2048.0f;
		shadowViewport.Height = 2048.0f;
		//shadowViewport.Width =  4096.0f;
		//shadowViewport.Height = 4096.0f;


		shadowViewport.MinDepth = 0.0f;
		shadowViewport.MaxDepth = 1.0f;
		shadowViewport.TopLeftX = 0.0f;
		shadowViewport.TopLeftY = 0.0f;

		D3D11_SAMPLER_DESC samplerDesc{};
		//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 線形フィルタ
		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER; // 範囲外は境界色
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.BorderColor[0] = 1.0f; // 範囲外は白（＝影にならない）にする
		samplerDesc.BorderColor[1] = 1.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;

		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		hr = device->CreateSamplerState(&samplerDesc, shadowSampler.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}


	// レンダーステート生成
	renderState = std::make_unique<RenderState>(device.Get());

	// レンダラ生成
	shapeRenderer = std::make_unique<ShapeRenderer>(device.Get());
	modelRenderer = std::make_unique<ModelRenderer>(device.Get());

	Microsoft::WRL::ComPtr<IDXGIDevice>   dxgiDevice;
	Microsoft::WRL::ComPtr<IDXGIAdapter>  dxgiAdapter;

	device->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf()));
	dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());

	DXGI_ADAPTER_DESC desc{};
	dxgiAdapter->GetDesc(&desc);

	// ワイド文字列をデバッグ出力
	OutputDebugStringW(desc.Description);
	OutputDebugStringW(L"\n");

	fullscreenQuad = std::make_unique<fullscreen_quad>(device.Get());
}

// クリア
void Graphics::Clear(float r, float g, float b, float a)
{
	float color[4]{ r, g, b, a };
	immediateContext->ClearRenderTargetView(renderTargetView.Get(), color);
	immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

// レンダーターゲット設定
void Graphics::SetRenderTargets()
{
	immediateContext->RSSetViewports(1, &viewport);
	immediateContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
}

// 画面表示
void Graphics::Present(UINT syncInterval)
{
	swapchain->Present(syncInterval, 0);
}

bool Graphics::CreateRenderTarget(RenderTarget& rt, int width, int height)
{
	ID3D11Device* device = this->device.Get();

	rt.width = width;
	rt.height = height;

	HRESULT hr;

	// =========================
	// カラーテクスチャ
	// =========================
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags =
		D3D11_BIND_RENDER_TARGET |
		D3D11_BIND_SHADER_RESOURCE;

	hr = device->CreateTexture2D(&texDesc, nullptr, rt.texture.GetAddressOf());
	if (FAILED(hr)) return false;

	// RTV
	hr = device->CreateRenderTargetView(rt.texture.Get(), nullptr, rt.rtv.GetAddressOf());
	if (FAILED(hr)) return false;

	// SRV（ImGui表示用）
	hr = device->CreateShaderResourceView(rt.texture.Get(), nullptr, rt.srv.GetAddressOf());
	if (FAILED(hr)) return false;

	// =========================
	// 深度バッファ
	// =========================
	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = device->CreateTexture2D(&depthDesc, nullptr, rt.depthTexture.GetAddressOf());
	if (FAILED(hr)) return false;

	hr = device->CreateDepthStencilView(rt.depthTexture.Get(), nullptr, rt.dsv.GetAddressOf());
	if (FAILED(hr)) return false;

	return true;
}

void Graphics::Blit(ID3D11ShaderResourceView* srv)
{
	auto* dc = immediateContext.Get();

	dc->OMSetDepthStencilState(
		renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
	dc->RSSetState(
		renderState->GetRasterizerState(RasterizerState::SolidCullNone));
	dc->OMSetBlendState(
		renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);

	fullscreenQuad->blit(dc, &srv, 0, 1, nullptr);
}

//影用バッファのクリア
void Graphics::ClearShadowMap()
{
	//影用DSVを最大値(1.0)で塗りつぶす
	immediateContext->ClearDepthStencilView(shadowMapDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}


//影描画の開始
void Graphics::BeginShadowMap()
{
	ClearShadowMap();
	viewport_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	immediateContext->RSGetViewports(&viewport_count, cached_viewports);
	immediateContext->OMGetRenderTargets(1, cached_render_target_view.ReleaseAndGetAddressOf(), cached_depth_stencil_view.ReleaseAndGetAddressOf());

	immediateContext->OMSetRenderTargets(0, renderTargetView.GetAddressOf(), shadowMapDSV.Get());
	//影用ビューポートをセット
	immediateContext->RSSetViewports(1, &shadowViewport);
}


void Graphics::EndShadowMap()
{
	immediateContext->RSSetViewports(viewport_count, cached_viewports);
	immediateContext->OMSetRenderTargets(1, cached_render_target_view.GetAddressOf(), cached_depth_stencil_view.Get());
}
