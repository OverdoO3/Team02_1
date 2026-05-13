#include <algorithm>
#include "Misc.h"
#include "GpuResourceUtils.h"
#include "ModelRenderer.h"
#include "BasicShader.h"
#include "LambertShader.h"
#include <imgui.h>

// コンストラクタ
ModelRenderer::ModelRenderer(ID3D11Device* device)
{
	// シーン用定数バッファ
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbScene),
		sceneConstantBuffer.GetAddressOf());

	// スケルトン用定数バッファ
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
}

// 描画実行
void ModelRenderer::Render(const RenderContext& rc, const DirectX::XMFLOAT4X4& worldTransform, const Model* model, ShaderId shaderId)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// シーン用定数バッファ更新
	{
		CbScene cbScene{};
		DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.view);
		DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.projection);
		DirectX::XMStoreFloat4x4(&cbScene.viewProjection, V * P);
		cbScene.lightDirection.x = rc.lightDirection.x;
		cbScene.lightDirection.y = rc.lightDirection.y;
		cbScene.lightDirection.z = rc.lightDirection.z;
		dc->UpdateSubresource(sceneConstantBuffer.Get(), 0, 0, &cbScene, 0, 0);
	}

	// 定数バッファ設定
	dc->VSSetConstantBuffers(1, 1, skeletonConstantBuffer.GetAddressOf());
	dc->VSSetConstantBuffers(3, 1, sceneConstantBuffer.GetAddressOf());
	dc->PSSetConstantBuffers(3, 1, sceneConstantBuffer.GetAddressOf());


	//ライト。影データ
	CbLight cbLight{};
	cbLight.lightViewProjection = rc.lightViewProjection;	//カメラ視点のWVP用
	cbLight.lightDirection = DirectX::XMFLOAT4(rc.lightDirection.x, rc.lightDirection.y, rc.lightDirection.z, 0.0f);
	cbLight.lightColor = rc.lightColor;
	cbLight.ambientColor = rc.ambientColor;
	memcpy_s(cbLight.point_light, sizeof(cbLight.point_light), point_light, sizeof(point_light));

	dc->UpdateSubresource(CbLightConstantBuffer.Get(), 0, 0, &cbLight, 0, 0);

	//ライト設定
	dc->VSSetConstantBuffers(2, 1, CbLightConstantBuffer.GetAddressOf());
	dc->PSSetConstantBuffers(2, 1, CbLightConstantBuffer.GetAddressOf());



	dc->UpdateSubresource(shadowParamsConstantBuffer.Get(), 0, 0, &rc.shadowParams, 0, 0);

	//lights.ambientColor = ambient_color;
	//lights.lightDirection = directional_light_direction;
	//lights.lightColor = directional_light_color;
	//memcpy_s(cbLight.point_light, sizeof(cbLight.point_light), point_light, sizeof(point_light));


	//CbLightParams cbLightParams{};
	//cbLightParams.lightIntensity;
	//cbLightParams.contrastPower = 1.0f;   
	//cbLightParams.padding1 = { 0.0f, 0.0f };

	dc->PSSetConstantBuffers(4, 1, shadowParamsConstantBuffer.GetAddressOf());
	dc->UpdateSubresource(CbLightParamsConstantBuffer.Get(), 0, 0, &rc.lightParams, 0, 0);
	dc->PSSetConstantBuffers(5, 1, CbLightParamsConstantBuffer.GetAddressOf());
	//ID3D11Buffer* psConstantBuffers[] =
	//{
	//	sceneConstantBuffer.Get(),
	//};
	//dc->PSSetConstantBuffers(0, _countof(psConstantBuffers), psConstantBuffers);

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

	// 描画処理
	//TODOShaderId変更
	//Shader* shader = shaders[static_cast<int>(shaderId)].get();
	//if (shaderId != ShaderId::Shadow)
	//{
	//	shader->Begin(rc);
	//}

	dc->IASetInputLayout(inputLayout.Get());

	if (shaderId == ShaderId::Outline)
	{
		dc->VSSetShader(outlineVertexShader.Get(), nullptr, 0);
		dc->PSSetShader(outlinePixelShader.Get(), nullptr, 0);

		dc->UpdateSubresource(outlineParamsConstantBuffer.Get(), 0, 0, &rc.outlineParams, 0, 0);
		dc->UpdateSubresource(outlineColorConstantBuffer.Get(), 0, 0, &rc.outlineColor, 0, 0);
		dc->VSSetConstantBuffers(6, 1, outlineParamsConstantBuffer.GetAddressOf());
		dc->PSSetConstantBuffers(7, 1, outlineColorConstantBuffer.GetAddressOf());

		dc->RSSetState(rc.renderState->GetCullFrontRasterizerState());
	}
	else if (shaderId == ShaderId::Shadow)
	{
		dc->VSSetShader(vertexShader.Get(), nullptr, 0);
		dc->PSSetShader(nullptr, nullptr, 0);
	}
	else
	{
		dc->VSSetShader(vertexShader.Get(), nullptr, 0);
		dc->PSSetShader(pixelShader.Get(), nullptr, 0);

		dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));
	}

	//if (shaderId != ShaderId::Shadow)
	//{
	//	dc->PSSetShader(pixelShader.Get(), nullptr, 0);
	//}


	DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&worldTransform);

	const ModelResource* resource = model->GetResource();
	for (const ModelResource::Mesh& mesh : resource->GetMeshes())
	{
		// 頂点バッファ設定
		UINT stride = sizeof(ModelResource::Vertex);
		UINT offset = 0;
		dc->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
		dc->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// スケルトン用定数バッファ更新
		CbSkeleton cbSkeleton{};
		if (mesh.nodeIndices.size() > 0)
		{
			for (size_t i = 0; i < mesh.nodeIndices.size(); ++i)
			{
				int nodeIndex = mesh.nodeIndices.at(i);
				const Model::Node& node = model->GetNodes().at(nodeIndex);
				DirectX::XMMATRIX GlobalTransform = DirectX::XMLoadFloat4x4(&node.globalTransform);
				DirectX::XMMATRIX OffsetTransform = DirectX::XMLoadFloat4x4(&mesh.offsetTransforms.at(i));
				DirectX::XMMATRIX BoneTransform = OffsetTransform * GlobalTransform * WorldTransform;
				DirectX::XMStoreFloat4x4(&cbSkeleton.boneTransforms[i], BoneTransform);
			}
		}
		else
		{
			const Model::Node& node = model->GetNodes().at(mesh.nodeIndex);
			DirectX::XMMATRIX GlobalTransform = DirectX::XMLoadFloat4x4(&node.globalTransform);
			DirectX::XMMATRIX BoneTransform = GlobalTransform * WorldTransform;
			DirectX::XMStoreFloat4x4(&cbSkeleton.boneTransforms[0], BoneTransform);
		}
		dc->UpdateSubresource(skeletonConstantBuffer.Get(), 0, 0, &cbSkeleton, 0, 0);

		// 描画
		for (const ModelResource::Subset& subset : mesh.subsets)
		{
			//if (shaderId != ShaderId::Shadow)
			//{
			//	shader->Update(rc, *subset.material);

			//}

			// シェーダーリソースビュー設定
			dc->PSSetShaderResources(0, 1, subset.material->shaderResourceView.GetAddressOf());

			dc->DrawIndexed(subset.indexCount, subset.startIndex, 0);

		}

	}

	//if (shaderId != ShaderId::Shadow)
	//{
	//	shader->End(rc);
	//}

	// 定数バッファ設定解除
	//for (ID3D11Buffer*& vsConstantBuffer : vsConstantBuffers) { vsConstantBuffer = nullptr; }
	//for (ID3D11Buffer*& psConstantBuffer : psConstantBuffers) { psConstantBuffer = nullptr; }
	//dc->VSSetConstantBuffers(0, _countof(vsConstantBuffers), vsConstantBuffers);
	//dc->PSSetConstantBuffers(0, _countof(psConstantBuffers), psConstantBuffers);

	//SRV解除
	ID3D11ShaderResourceView* null_srv{ NULL };
	dc->PSSetShaderResources(8, 1, &null_srv);
}

void ModelRenderer::DebugImGui()
{

}