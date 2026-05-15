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

class Actor;

enum class ShaderId
{
	Basic,
	Lambert,

	EnumCount
};

class ModelRenderer
{
public:
    ModelRenderer(ID3D11Device* device);

    void AddInstance(const Model* model, Actor* actor);
    void FlushAll(const RenderContext& rc);

    struct InstanceData
    {
        DirectX::XMFLOAT4X4 world;
    };
private:
    struct CbScene
    {
        DirectX::XMFLOAT4X4 viewProjection;
        DirectX::XMFLOAT4 lightDirection;
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

    static constexpr int MAX_INSTANCES = 8192;

    std::unique_ptr<Shader> shaders[static_cast<int>(ShaderId::EnumCount)];

    Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> skeletonConstantBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer> instanceBuffer;

    ShaderId batchShader = ShaderId::Lambert;

    ID3D11ShaderResourceView* instanceBufferSRV = nullptr;
public:

    std::unordered_map<const ModelResource*, BatchData> batches;
};