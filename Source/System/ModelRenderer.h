#pragma once

#include <memory>
#include <vector>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Model.h"
#include "Shader.h"
#include "../System/ShapeRenderer.h"

enum class ShaderId
{
	Basic,
	Lambert,
	Shadow,
	EnumCount,
	Outline,
};



class ModelRenderer
{
public:
	struct CbLight
	{
		DirectX::XMFLOAT4X4 lightViewProjection;
		DirectX::XMFLOAT4 lightDirection;
		DirectX::XMFLOAT4 lightColor;
		DirectX::XMFLOAT4 ambientColor;
		point_lights point_light[8];
	};
	ModelRenderer(ID3D11Device* device);
	~ModelRenderer() {}

	// •`‰æŽÀs
	void Render(const RenderContext& rc, const DirectX::XMFLOAT4X4& worldTransform, const Model* model, ShaderId shaderId);

	void DebugImGui();
	point_lights* GetPointLights() { return point_light; }

	void UpdateLightInner(ID3D11DeviceContext* dc, const CbLight& lightData) const {
		dc->PSSetConstantBuffers(2, 1, CbLightConstantBuffer.GetAddressOf());
		dc->UpdateSubresource(CbLightConstantBuffer.Get(), 0, nullptr, &lightData, 0, 0);
	}


private:
	struct CbScene
	{
		DirectX::XMFLOAT4X4		viewProjection;
		DirectX::XMFLOAT4		lightDirection;

		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMFLOAT4 lightColor;
		DirectX::XMFLOAT4 cameraPosition;
		DirectX::XMFLOAT4X4 lightViewProjection;
	};

	struct CbSkeleton
	{
		DirectX::XMFLOAT4X4		boneTransforms[256];
	};


	std::unique_ptr<Shader>					shaders[static_cast<int>(ShaderId::EnumCount)];

	Microsoft::WRL::ComPtr<ID3D11Buffer>	sceneConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>    lightConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	CbLightConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	skeletonConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	shadowParamsConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	CbLightParamsConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>    outlineParamsConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>    outlineColorConstantBuffer;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> outlineVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  outlinePixelShader;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> cullFrontRasterizerState;

	CbLight m_lightConstants;
	point_lights point_light[8];
};
