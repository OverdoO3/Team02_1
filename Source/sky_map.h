#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>
#include "System/RenderContext.h"

class sky_map
{
public:
	sky_map(ID3D11Device* device, const char* filename, bool generate_mips = false);
	virtual ~sky_map() = default;
	sky_map(const sky_map&) = delete;
	sky_map& operator =(const sky_map&) = delete;
	sky_map(sky_map&&) noexcept = delete;
	sky_map& operator =(sky_map&&) noexcept = delete;

	void blit(const RenderContext& rc, const DirectX::XMFLOAT4X4& view_projection,const DirectX::XMFLOAT4& cameraPosition);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> sky_map_vs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> sky_map_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> sky_box_ps;

	struct constants
	{
		DirectX::XMFLOAT4X4 inverse_view_projection;
		DirectX::XMFLOAT4 camera_position;
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;

	bool is_texturecube = false;

};