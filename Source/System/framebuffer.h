#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

class framebuffer
{
public:
	//width, height : 作成するテクスチャのサイズ
	//has_depthstancil : ブルームの抽出用などで奥行きが必要ならtrueにする
	framebuffer(ID3D11Device* device, uint32_t width, uint32_t height, bool has_depthstanvil = false);
	virtual ~framebuffer() = default;

	//バッファを書き込むためのView
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
	//奥行き判定用のView
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view;
	// シェーダーに渡すためのView
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_views[2];

	//このバッファ専用のビューポート
	D3D11_VIEWPORT viewport;

	//バッファをクリア
	void clear(ID3D11DeviceContext* immediate_context, float r = 0, float g = 0, float b = 0, float a = 1, float depth = 1);

	//書き込み先をこのバッファに切り替え
	void activate(ID3D11DeviceContext* immediate_context);

	//書き込み先を元(メイン画面)に戻す
	void deactivate(ID3D11DeviceContext* immediate_context);

private:
	//元のレンダーターゲットを一時的に保存するための変数
	UINT viewport_count{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
	D3D11_VIEWPORT cached_viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cached_render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cached_depth_stencil_view;
};