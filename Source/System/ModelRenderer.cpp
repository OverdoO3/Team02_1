#include "ModelRenderer.h"
#include "GpuResourceUtils.h"
#include "Actor.h"
#include "ModelResource.h"
#include "Shader.h"
#include <DirectXMath.h>
#include "BasicShader.h"
#include "LambertShader.h"
#include <imgui.h>

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

	//GpuResourceUtils::CreateConstantBuffer(
	//	device,
	//	sizeof(CbSkeleton),
	//	lightConstantBuffer.GetAddressOf());


	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbShadowParams),
		shadowParamsConstantBuffer.GetAddressOf());

	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbLightParams),
		CbLightParamsConstantBuffer.GetAddressOf());

	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbLight),
		CbLightConstantBuffer.GetAddressOf());

	// 頂点シェーダー
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/LambertVS.cso",
		ModelResource::InputElementDescs.data(),
		static_cast<UINT>(ModelResource::InputElementDescs.size()),
		inputLayout.GetAddressOf(),
		vertexShader.GetAddressOf());

	// ピクセルシェーダー
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/LambertPS.cso",
		pixelShader.GetAddressOf());

	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbOutlineParams),
		outlineParamsConstantBuffer.GetAddressOf());

	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbOutlineColor),
		outlineColorConstantBuffer.GetAddressOf());

	// アウトライン用頂点シェーダー
	GpuResourceUtils::LoadVertexShader(
		device,
		"./Data/Shader/out_line_vs.cso",
		ModelResource::InputElementDescs.data(),
		static_cast<UINT>(ModelResource::InputElementDescs.size()),
		nullptr,
		outlineVertexShader.GetAddressOf());

	// アウトライン用ピクセルシェーダー
	GpuResourceUtils::LoadPixelShader(
		device,
		"./Data/Shader/out_line_ps.cso",
		outlinePixelShader.GetAddressOf());

    //コイン半透明用
    GpuResourceUtils::LoadPixelShader(
        device,
        "Data/Shader/DitherPS.cso",
        ditherPixelShader.GetAddressOf());

	// シェーダー生成
	shaders[static_cast<int>(ShaderId::Basic)] = std::make_unique<BasicShader>(device);
	shaders[static_cast<int>(ShaderId::Lambert)] = std::make_unique<LambertShader>(device);

	{
		point_light[0].position.x = 10;
		point_light[0].position.y = 1;
		point_light[0].range = 100;
		point_light[0].color = { 1, 1, 1, 1 };

		point_light[1].position.x = -10;
		point_light[1].position.y = 1;
		point_light[1].range = 10;
		point_light[1].color = { 0, 1, 0, 1 };

		point_light[2].position.y = 1;
		point_light[2].position.z = 10;
		point_light[2].range = 10;
		point_light[2].position.y = 1;
		point_light[2].color = { 0, 0, 1, 1 };

		point_light[3].position.y = 1;
		point_light[3].position.z = -10;
		point_light[3].range = 10;
		point_light[3].color = { 1, 1, 1, 1 };

		point_light[4].range = 10;
		point_light[4].color = { 1, 1, 1, 1 };
		ZeroMemory(&point_light[5], sizeof(point_lights) * 3);
	}
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

    {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = sizeof(CbDither);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        hr = device->CreateBuffer(&desc, nullptr, ditherConstantBuffer.GetAddressOf());
        assert(SUCCEEDED(hr));
    }

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
    auto* t = actor->GetComponent<Transform>();
    if (!t) return;

    InstanceData data;
    XMStoreFloat4x4(&data.world, XMLoadFloat4x4(&t->GetWorldMatrix()));

    auto* mr = actor->GetComponent<ModelRender>();
    if (mr)
    {
        data.overrideSRV = mr->overrideSRV.Get();
        data.shaderId = mr->GetShaderId();
        data.ditherAlpha = mr->GetDitherAlpha();
    }

    batches[model->GetResource()].instances.push_back(data);
    batches[model->GetResource()].representativeModel = model;
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

    //=====================
    // 定数バッファ設定
    //=====================
    dc->VSSetConstantBuffers(1, 1, skeletonConstantBuffer.GetAddressOf());
    dc->VSSetConstantBuffers(3, 1, sceneConstantBuffer.GetAddressOf());
    dc->PSSetConstantBuffers(3, 1, sceneConstantBuffer.GetAddressOf());

    //=====================
    // ライト・影データ
    //=====================
    {
        CbLight cbLight{};
        cbLight.lightViewProjection = rc.lightViewProjection;
        cbLight.lightDirection = DirectX::XMFLOAT4(
            rc.lightDirection.x, rc.lightDirection.y, rc.lightDirection.z, 0.0f);
        cbLight.lightColor = rc.lightColor;
        cbLight.ambientColor = rc.ambientColor;
        memcpy_s(cbLight.point_light, sizeof(cbLight.point_light),
            point_light, sizeof(point_light));

        dc->UpdateSubresource(CbLightConstantBuffer.Get(), 0, nullptr, &cbLight, 0, 0);
        dc->VSSetConstantBuffers(2, 1, CbLightConstantBuffer.GetAddressOf());
        dc->PSSetConstantBuffers(2, 1, CbLightConstantBuffer.GetAddressOf());
    }

    //=====================
    // 影パラメータ
    //=====================
    dc->UpdateSubresource(shadowParamsConstantBuffer.Get(), 0, nullptr, &rc.shadowParams, 0, 0);
    dc->PSSetConstantBuffers(4, 1, shadowParamsConstantBuffer.GetAddressOf());

    //=====================
    // ライトパラメータ
    //=====================
    dc->UpdateSubresource(CbLightParamsConstantBuffer.Get(), 0, nullptr, &rc.lightParams, 0, 0);
    dc->PSSetConstantBuffers(5, 1, CbLightParamsConstantBuffer.GetAddressOf());

    //=====================
    // サンプラステート設定
    //=====================
    ID3D11SamplerState* samplerStates[] =
    {
        rc.renderState->GetSamplerState(SamplerState::LinearWrap)
    };
    dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);

    //=====================
    // レンダーステート設定
    //=====================
    dc->OMSetDepthStencilState(
        rc.renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
    dc->RSSetState(
        rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));

    //=====================
    // ブレンドステート設定
    //=====================
    dc->OMSetBlendState(
        rc.renderState->GetBlendState(BlendState::Transparency), nullptr, 0xFFFFFFFF);

    //=====================
    // InputLayout & Shader 切り替え（引数の shaderId で判定）
    //=====================
    dc->IASetInputLayout(inputLayout.Get());

    if (shaderId == ShaderId::Outline)
    {
        dc->VSSetShader(outlineVertexShader.Get(), nullptr, 0);
        dc->PSSetShader(outlinePixelShader.Get(), nullptr, 0);

        dc->UpdateSubresource(outlineParamsConstantBuffer.Get(), 0, nullptr, &rc.outlineParams, 0, 0);
        dc->UpdateSubresource(outlineColorConstantBuffer.Get(), 0, nullptr, &rc.outlineColor, 0, 0);
        dc->VSSetConstantBuffers(6, 1, outlineParamsConstantBuffer.GetAddressOf());
        dc->PSSetConstantBuffers(7, 1, outlineColorConstantBuffer.GetAddressOf());

        dc->RSSetState(rc.renderState->GetCullFrontRasterizerState());
    }
    else if (shaderId == ShaderId::Shadow)
    {
        dc->VSSetShader(vertexShader.Get(), nullptr, 0);
        dc->PSSetShader(nullptr, nullptr, 0);
    }
    else if (shaderId == ShaderId::Dither)
    {
        dc->VSSetShader(vertexShader.Get(), nullptr, 0);
        dc->PSSetShader(ditherPixelShader.Get(), nullptr, 0);

        CbDither cbDither;
        cbDither.ditherAlpha = m_ditherAlpha;

        dc->UpdateSubresource(ditherConstantBuffer.Get(), 0, nullptr, &cbDither, 0, 0);
        dc->PSSetConstantBuffers(6, 1, ditherConstantBuffer.GetAddressOf());

        dc->OMSetDepthStencilState(
            rc.renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
        dc->RSSetState(
            rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));
        dc->OMSetBlendState(
            rc.renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
    }
    else
    {
        // Lambert など通常シェーダー
        Shader* shader = shaders[(int)ShaderId::Lambert].get();
        shader->Begin(rc);

        dc->VSSetShader(vertexShader.Get(), nullptr, 0);
        dc->PSSetShader(pixelShader.Get(), nullptr, 0);
        dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));
    }

    //=====================
    // モデル単位（バッチインスタンシング）
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
        // InstanceBuffer 更新
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

        UINT instanceCount = (UINT)instances.instances.size();

        //=====================
        // Mesh 単位
        //=====================
        for (const auto& mesh : resource->GetMeshes())
        {
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
            // Skeleton CB 更新
            //=====================
            CbSkeleton cbSkel{};
            if (!mesh.nodeIndices.empty())
            {
                for (size_t i = 0; i < mesh.nodeIndices.size(); i++)
                {
                    const Model::Node& node =
                        instances.representativeModel->GetNodes().at(mesh.nodeIndices[i]);
                    DirectX::XMMATRIX G = DirectX::XMLoadFloat4x4(&node.globalTransform);
                    DirectX::XMMATRIX O = DirectX::XMLoadFloat4x4(&mesh.offsetTransforms[i]);
                    DirectX::XMStoreFloat4x4(&cbSkel.boneTransforms[i], O * G);
                }
            }
            else
            {
                const Model::Node& node =
                    instances.representativeModel->GetNodes().at(mesh.nodeIndex);
                DirectX::XMMATRIX G = DirectX::XMLoadFloat4x4(&node.globalTransform);
                DirectX::XMStoreFloat4x4(&cbSkel.boneTransforms[0], G);
            }
            dc->UpdateSubresource(skeletonConstantBuffer.Get(), 0, nullptr, &cbSkel, 0, 0);

            //=====================
            // Draw（インスタンシング）
            //=====================
            const ModelResource::Material* lastMaterial = nullptr;

            for (const auto& subset : mesh.subsets)
            {
                if (shaderId != ShaderId::Shadow)
                {
                    if (subset.material != lastMaterial)
                    {
                        // ★ overrideSRV があれば差し替え、なければ通常テクスチャ
                        ID3D11ShaderResourceView* srv =
                            instances.instances[0].overrideSRV
                            ? instances.instances[0].overrideSRV
                            : subset.material->shaderResourceView.Get();

                        // Dither シェーダー用のマテリアル単位パラメータ更新
                        if (shaderId == ShaderId::Dither)
                        {
                            CbDither cbDither{};
                            cbDither.ditherAlpha = m_ditherAlpha;

                            dc->UpdateSubresource(ditherConstantBuffer.Get(), 0, nullptr, &cbDither, 0, 0);
                            dc->PSSetConstantBuffers(6, 1, ditherConstantBuffer.GetAddressOf());
                        }

                        // 共通: テクスチャをピクセルシェーダにバインド
                        dc->PSSetShaderResources(0, 1, &srv);

                        lastMaterial = subset.material;
                    }
                }

                dc->DrawIndexedInstanced(
                    subset.indexCount,
                    instanceCount,
                    subset.startIndex,
                    0,
                    0);
            }
        }
    }

    //=====================
    // Lambert シェーダー終了処理
    //=====================
    if (shaderId != ShaderId::Shadow && shaderId != ShaderId::Outline)
    {
        Shader* shader = shaders[(int)ShaderId::Lambert].get();
        shader->End(rc);
    }

    //=====================
    // リソース解除（クリーンアップ）
    //=====================
    ID3D11ShaderResourceView* nullSRVs[] = { nullptr };
    dc->VSSetShaderResources(0, 1, nullSRVs);

    // Shadow SRV は Shadow パス以外では解除しない（通常描画で使うため）
    if (shaderId == ShaderId::Shadow)
    {
        dc->PSSetShaderResources(8, 1, nullSRVs);
    }

    {
        ID3D11Buffer* nullVSCBs[] = { nullptr, nullptr, nullptr, nullptr };
        dc->VSSetConstantBuffers(0, _countof(nullVSCBs), nullVSCBs);
    }
    {
        ID3D11Buffer* nullPSCBs[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
        dc->PSSetConstantBuffers(0, _countof(nullPSCBs), nullPSCBs);
    }

    for (ID3D11SamplerState*& s : samplerStates) { s = nullptr; }
    dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);

    // バッチクリア（メモリは保持）
    for (auto& [resource, batch] : batches)
    {
        batch.instances.clear();
        batch.representativeModel = nullptr;
    }
}

#ifdef _DEBUG
void ModelRenderer::DebugImGui()
{
    if (ImGui::Begin("PointLights Debug"))
    {
        ImGui::Text("Current Light File: %s", currentLightPath.c_str());

        if (ImGui::Button("Save Lights"))
            SaveLights(currentLightPath);

        ImGui::SameLine();

        if (ImGui::Button("Load Lights"))
            LoadLights(currentLightPath);

        for (int i = 0; i < 8; i++)
        {
            ImGui::PushID(i);
            if (ImGui::CollapsingHeader(("Light[" + std::to_string(i) + "]").c_str()))
            {
                ImGui::SliderFloat3("Position", &point_light[i].position.x, -150.0f, 150.0f);
                ImGui::SliderFloat("Range", &point_light[i].range, 0.0f, 200.0f);
                ImGui::ColorEdit4("Color", &point_light[i].color.x);
            }
            ImGui::PopID();
        }
    }
    ImGui::End();
}
#endif // _DEBUG


void ModelRenderer::SaveLights(const std::string& path) 
{
    nlohmann::json j;
    for (int i = 0; i < 8; i++)
    {
        j["lights"][i]["posX"] = point_light[i].position.x;
        j["lights"][i]["posY"] = point_light[i].position.y;
        j["lights"][i]["posZ"] = point_light[i].position.z;
        j["lights"][i]["range"] = point_light[i].range;
        j["lights"][i]["colorR"] = point_light[i].color.x;
        j["lights"][i]["colorG"] = point_light[i].color.y;
        j["lights"][i]["colorB"] = point_light[i].color.z;
        j["lights"][i]["colorA"] = point_light[i].color.w;
    }
    std::ofstream ofs(path);
    ofs << j.dump(4);
}

void ModelRenderer::LoadLights(const std::string& path)
{
    std::ifstream ifs(path);
    if (!ifs) return;
    nlohmann::json j;
    ifs >> j;
    for (int i = 0; i < 8; i++)
    {
        point_light[i].position.x = j["lights"][i].value("posX", 0.0f);
        point_light[i].position.y = j["lights"][i].value("posY", 0.0f);
        point_light[i].position.z = j["lights"][i].value("posZ", 0.0f);
        point_light[i].range = j["lights"][i].value("range", 0.0f);
        point_light[i].color.x = j["lights"][i].value("colorR", 0.0f);
        point_light[i].color.y = j["lights"][i].value("colorG", 0.0f);
        point_light[i].color.z = j["lights"][i].value("colorB", 0.0f);
        point_light[i].color.w = j["lights"][i].value("colorA", 1.0f);
    }
}

void ModelRenderer::SetLightEnabled(int index, bool enabled)
{
    if (index < 0 || index >= 8) return;

    if (enabled) {
        // オンにする時は保存しておいたrangeを復元
        point_light[index].range = point_light[index].savedRange;
        point_light[index].isEnabled = true;
    }
    else {
        // オフにする時は現在のrangeを保存して0にする
        point_light[index].savedRange = point_light[index].range;
        point_light[index].range = 0.0f;
        point_light[index].isEnabled = false;
    }
}