#include "sky_map.h"
#include "System/GpuResourceUtils.h"
#include "System/Misc.h"

sky_map::sky_map(ID3D11Device* device, const char* filename, bool generate_mips)
{
	D3D11_TEXTURE2D_DESC texture2d_desc;
	GpuResourceUtils::LoadTexture(device,filename,shader_resource_view.GetAddressOf(),&texture2d_desc);

	if (texture2d_desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
	{
		is_texturecube = true;
	}

	GpuResourceUtils::LoadVertexShader(device, "./Data/Shader/sky_map_vs.cso", NULL, 0, NULL, sky_map_vs.GetAddressOf());
	GpuResourceUtils::LoadPixelShader(device, "./Data/Shader/sky_map_ps.cso", sky_map_ps.GetAddressOf());
	GpuResourceUtils::LoadPixelShader(device, "./Data/Shader/sky_box_ps.cso", sky_box_ps.GetAddressOf());

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	HRESULT hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

void sky_map::blit(const RenderContext& rc, const DirectX::XMFLOAT4X4& view_projection, const DirectX::XMFLOAT4& cameraPosition)
{
	// サンプラステート設定
	ID3D11SamplerState* samplerStates[] =
	{
		nullptr,
		rc.renderState->GetSamplerState(SamplerState::LinearWrap)
	};
	rc.deviceContext->PSSetSamplers(0, _countof(samplerStates), samplerStates);

	// レンダーステート設定
	rc.deviceContext->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestOnly), 0);
	//dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));

	// ブレンドステート設定
	rc.deviceContext->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);


	rc.deviceContext->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	rc.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	rc.deviceContext->IASetInputLayout(NULL);

	rc.deviceContext->VSSetShader(sky_map_vs.Get(), 0, 0);
	rc.deviceContext->PSSetShader(is_texturecube ? sky_box_ps.Get() : sky_map_ps.Get(), 0, 0);

	rc.deviceContext->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());

	constants data;
	DirectX::XMStoreFloat4x4(&data.inverse_view_projection, DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&view_projection)));
	data.camera_position = DirectX::XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);

	rc.deviceContext->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
	rc.deviceContext->PSSetConstantBuffers(0, 1, constant_buffer.GetAddressOf());

	rc.deviceContext->Draw(4, 0);

	rc.deviceContext->VSSetShader(NULL, 0, 0);
	rc.deviceContext->PSSetShader(NULL, 0, 0);
}
