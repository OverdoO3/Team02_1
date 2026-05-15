#include "ModelRenderer.h"
#include "GpuResourceUtils.h"
#include "Actor.h"
#include "ModelResource.h"
#include "Shader.h"
#include <DirectXMath.h>
#include "BasicShader.h"
#include "LambertShader.h"

using namespace DirectX;

//==============================
// コンストラクタ
//==============================
ModelRenderer::ModelRenderer(ID3D11Device* device)
{
    // Scene CB
    GpuResourceUtils::CreateConstantBuffer(
        device,
        sizeof(CbScene),
        sceneConstantBuffer.GetAddressOf());

    // Skeleton CB
    GpuResourceUtils::CreateConstantBuffer(
        device,
        sizeof(CbSkeleton),
        skeletonConstantBuffer.GetAddressOf());

    // Instance Buffer（StructuredではなくVertexBuffer運用）
   /* GpuResourceUtils::CreateDynamicVertexBuffer(
        device,
        sizeof(InstanceData) * MAX_INSTANCES,
        instanceBuffer.GetAddressOf());*/

    // Shaders
    shaders[(int)ShaderId::Basic] =
        std::make_unique<BasicShader>(device);

    shaders[(int)ShaderId::Lambert] =
        std::make_unique<LambertShader>(device);

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(InstanceData) * MAX_INSTANCES;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    HRESULT hr = device->CreateBuffer(&desc, nullptr, instanceBuffer.GetAddressOf());
    assert(SUCCEEDED(hr));

    //D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    //srvDesc.Format = DXGI_FORMAT_UNKNOWN;      // StructuredBufferはUNKNOWN必須
    //srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    //srvDesc.Buffer.FirstElement = 0;
    //srvDesc.Buffer.NumElements = MAX_INSTANCES;

    //hr = device->CreateShaderResourceView(
    //    instanceBuffer.Get(), &srvDesc, &instanceBufferSRV);
    //assert(SUCCEEDED(hr));
}

//==============================
// AddInstance（バッチ登録）
//==============================
void ModelRenderer::AddInstance(const Model* model, Actor* actor)
{
    auto t = actor->GetComponent<Transform>();
    if (!t) return;

    InstanceData data{};
    data.world = t->GetWorldMatrix();
    
    auto& batch = batches[model->GetResource()];
    batch.representativeModel = model;
    batch.instances.push_back(data);
}

//==============================
// FlushAll（描画本体）
//==============================

void ModelRenderer::FlushAll(const RenderContext& rc)
{
    if (batches.empty())
        return;

    auto* dc = rc.deviceContext;

    //=====================
    // Scene CB（1回）
    //=====================
    {
        CbScene cb{};
        auto V = XMLoadFloat4x4(&rc.view);
        auto P = XMLoadFloat4x4(&rc.projection);
        XMStoreFloat4x4(&cb.viewProjection, V * P);

        cb.lightDirection.x = rc.lightDirection.x;
        cb.lightDirection.y = rc.lightDirection.y;
        cb.lightDirection.z = rc.lightDirection.z;

        dc->UpdateSubresource(sceneConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);
    }

    // 定数バッファ設定
    ID3D11Buffer* vsConstantBuffers[] =
    {
        sceneConstantBuffer.Get(),
        skeletonConstantBuffer.Get(),
    };
    dc->VSSetConstantBuffers(0, _countof(vsConstantBuffers), vsConstantBuffers);

    ID3D11Buffer* psConstantBuffers[] =
    {
        sceneConstantBuffer.Get(),
    };
    dc->PSSetConstantBuffers(0, _countof(psConstantBuffers), psConstantBuffers);

    // サンプラステート設定
    ID3D11SamplerState* samplerStates[] =
    {
        rc.renderState->GetSamplerState(SamplerState::LinearWrap)
    };
    dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);

    // レンダーステート設定
    dc->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
    dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));

    // ブレンドステート設定
    dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Transparency), nullptr, 0xFFFFFFFF);

    Shader* shader = shaders[(int)ShaderId::Lambert].get();
    shader->Begin(rc);

    //=====================
    // モデル単位
    //=====================
    for (auto& [resource, instances] : batches)
    {
        if (instances.instances.empty()) continue;

        if (instances.instances.size() > MAX_INSTANCES)
        {
            // ★ ここに来たら MAX_INSTANCES を増やす必要がある
            assert(false && "MAX_INSTANCES exceeded");
        }
        //=====================
        // InstanceBuffer更新
        //=====================
        {
            D3D11_MAPPED_SUBRESOURCE mapped{};
            HRESULT hr = dc->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            assert(SUCCEEDED(hr));
            memcpy(mapped.pData,
                instances.instances.data(),
                sizeof(InstanceData) * instances.instances.size());

            dc->Unmap(instanceBuffer.Get(), 0);
        }
            //ID3D11ShaderResourceView* vsSRVs[] = { instanceBufferSRV };
            //dc->VSSetShaderResources(0, 1, vsSRVs);

            UINT instanceCount = (UINT)instances.instances.size();
    
        //=====================
        // Mesh単位
        //=====================
        for (const auto& mesh : resource->GetMeshes())
        {
            UINT stride = sizeof(ModelResource::Vertex);
            UINT offset = 0;

            ID3D11Buffer* buffers[2] =
            {
                mesh.vertexBuffer.Get(),
                instanceBuffer.Get()
            };

            UINT strides[2] =
            {
                sizeof(ModelResource::Vertex),
                sizeof(InstanceData)
            };

            UINT offsets[2] = { 0, 0 };

            dc->IASetVertexBuffers(0, 2, buffers, strides, offsets);
            dc->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            //=====================
            // Skeleton
            //=====================

            CbSkeleton cbSkel{};
            if (!mesh.nodeIndices.empty())
            {
                for (size_t i = 0; i < mesh.nodeIndices.size(); i++)
                {
                    const Model::Node& node = instances.representativeModel->GetNodes().at(mesh.nodeIndices[i]);
                    DirectX::XMMATRIX G = DirectX::XMLoadFloat4x4(&node.globalTransform);
                    DirectX::XMMATRIX O = DirectX::XMLoadFloat4x4(&mesh.offsetTransforms[i]);
                    DirectX::XMStoreFloat4x4(&cbSkel.boneTransforms[i], O * G);
                }
            }
            else
            {
                const Model::Node& node = instances.representativeModel->GetNodes().at(mesh.nodeIndex);
                DirectX::XMMATRIX G = DirectX::XMLoadFloat4x4(&node.globalTransform);
                DirectX::XMStoreFloat4x4(&cbSkel.boneTransforms[0], G);
            }
            // GPUへ送る
            dc->UpdateSubresource(skeletonConstantBuffer.Get(), 0, nullptr, &cbSkel, 0, 0);

            //=====================
            // Draw
            //=====================

            const ModelResource::Material* lastMaterial = nullptr;

            for (const auto& subset : mesh.subsets)
            {
                if (subset.material != lastMaterial)
                {
                    shader->Update(rc, *subset.material);
                    lastMaterial = subset.material;
                }
                //dc->DrawIndexed(subset.indexCount, subset.startIndex, 0);
                dc->DrawIndexedInstanced(
                    subset.indexCount,   // IndexCountPerInstance
                    instanceCount,       // InstanceCount
                    subset.startIndex,   // StartIndexLocation
                    0,                   // BaseVertexLocation
                    0);                  // StartInstanceLocation
            }
        }
    }

    shader->End(rc);

    ID3D11ShaderResourceView* nullSRVs[] = { nullptr };
    dc->VSSetShaderResources(0, 1, nullSRVs);

    for (ID3D11Buffer*& vsConstantBuffer : vsConstantBuffers) { vsConstantBuffer = nullptr; }
    for (ID3D11Buffer*& psConstantBuffer : psConstantBuffers) { psConstantBuffer = nullptr; }
    dc->VSSetConstantBuffers(0, _countof(vsConstantBuffers), vsConstantBuffers);
    dc->PSSetConstantBuffers(0, _countof(psConstantBuffers), psConstantBuffers);

    // サンプラステート設定解除
    for (ID3D11SamplerState*& samplerState : samplerStates) { samplerState = nullptr; }
    dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);

    // cleanup
    for (auto& [resource, batch] : batches)
    {
        batch.instances.clear(); // ★ vector のメモリは解放しない
        batch.representativeModel = nullptr;
    }
}