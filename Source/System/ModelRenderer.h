#pragma once

#include <memory>
#include <vector>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Model.h"
#include "Shader.h"
#include "ModelResource.h"
#include <map>
#include <unordered_map>
#include "../System/ShapeRenderer.h"
#include <fstream>
#include <filesystem>

class Actor;

enum class ShaderId
{
	Basic,
	Lambert,
	Shadow,
	EnumCount,
	Outline,
	Dither,
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

    void AddInstance(const Model* model, Actor* actor);
    void FlushAll(const RenderContext& rc);

	void DebugImGui();
	point_lights* GetPointLights() { return point_light; }

	void UpdateLightInner(ID3D11DeviceContext* dc, const CbLight& lightData) const {
		dc->PSSetConstantBuffers(2, 1, CbLightConstantBuffer.GetAddressOf());
		dc->UpdateSubresource(CbLightConstantBuffer.Get(), 0, nullptr, &lightData, 0, 0);
	}

    struct InstanceData
    {
        DirectX::XMFLOAT4X4 world;
		ID3D11ShaderResourceView* overrideSRV = nullptr;
		//float ditherAlpha; 
		//float padding[3];  
    };

	void SetShaderId(ShaderId id) { shaderId = id; }
	ShaderId GetShaderId() const { return shaderId; }

	void SaveLights(const std::string& path);
	void LoadLights(const std::string& path);

	void SetLightEnabled(int index, bool enabled);

	std::string currentLightPath = "Data/Lights/default.json";

	void SetLightPath(const std::string& scenePath)
	{
		// "Scenes/stage1.json" Å® "Data/Lights/stage1_lights.json"
		std::filesystem::path p(scenePath);
		currentLightPath = "Data/Lights/" + p.stem().string() + "_lights.json";
	}
	std::string GetCurrentLightPath() const { return currentLightPath; }
	void SetDitherAlpha(float alpha) { m_ditherAlpha = alpha; }
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
        DirectX::XMFLOAT4X4 boneTransforms[256];
    };

    struct BatchData
    {
        const Model* representativeModel = nullptr;
        std::vector<InstanceData> instances;
    };

	struct CbDither
	{
		float ditherAlpha;
		DirectX::XMFLOAT3 padding;
	};

    static constexpr int MAX_INSTANCES = 8192;

    std::unique_ptr<Shader> shaders[static_cast<int>(ShaderId::EnumCount)];

    Microsoft::WRL::ComPtr<ID3D11Buffer> instanceBuffer;

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
	Microsoft::WRL::ComPtr<ID3D11Buffer> ditherConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ditherPixelShader;

	ShaderId shaderId = ShaderId::Lambert;

	CbLight m_lightConstants;
	point_lights point_light[8];
    ShaderId batchShader = ShaderId::Lambert;
public:
	ID3D11ShaderResourceView* instanceBufferSRV = nullptr;
    int debugInstanceCount = 0;
    std::unordered_map<const ModelResource*, BatchData> batches;
	float m_ditherAlpha = 0.5f;

};