#include "Scene.h"
#include "SceneManager.h"
#include "EffectManager.h"
#include "OpenDialog.h"

void Scene::Initialize(const char* path)
{
	SceneSerializer::Load(*this,path);
	InitializeAfterLoad();
}

void Scene::Update(float elapsedTime)
{
	if(playState)
	{
		//アウェイク
		for (auto& a : actors)
		{
			bool active = a->setActive && (!a->GetParent() || a->GetParent()->setActive);
			if (active)
			{
				a->OnAwake(elapsedTime);
			}
		}

		///アップデート
		for (auto& a : actors)
		{
			bool active = a->setActive && (!a->GetParent() || a->GetParent()->setActive);
			if (active)
			{
				a->Update(elapsedTime);
			}
		}

		//アクターの追加
		for (auto& a : adderActors)
		{
			actors.push_back(std::move(a));
		}

		actors.erase(
			std::remove_if(actors.begin(), actors.end(),
				[](const std::unique_ptr<Actor>& a)
				{
					return a->isDead;
				}),
			actors.end()
		);

		physics.Flush();
		adderActors.clear();
	}
	else
	{
		for (auto& a : actors)
		{
			bool active = a->setActive && (!a->GetParent() || a->GetParent()->setActive);
			if (active)
			{
				a->UpdateWithOutPlayed(elapsedTime);
			}
		}
		//アクターの追加
		for (auto& a : adderActors)
		{
			actors.push_back(std::move(a));
		}

		actors.erase(
			std::remove_if(actors.begin(), actors.end(),
				[](const std::unique_ptr<Actor>& a)
				{
					return a->isDead;
				}),
			actors.end()
		);

		physics.Flush();
		adderActors.clear();
	}
#ifdef _DEBUG
    if (ImGui::Begin("Debug Settings"))
    {
        if (ImGui::CollapsingHeader("Light & Shadow", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // ライト方向
            ImGui::DragFloat3("Light Direction", &lightDir.x, 0.01f, -1.0f, 1.0f);

            ImGui::SliderFloat("Shadow Range", &shadowRange, 0.1f, 200.0f);

            ImGui::Checkbox("Follow Player", &followPlayer);

            // 色設定
            ImGui::ColorEdit4("Light Color", &lightCol.x);
            ImGui::ColorEdit4("Ambient Color", &ambientCol.x);

            ImGui::Text("Shadow editer");
            ImGui::SliderFloat("Shadow Alpha", &m_shadowParams.shadow_color, 0.0f, 1.0f);
            ImGui::DragFloat("Shadow Bias", &m_shadowParams.shadow_bias, 0.0001f, 0.0f, 0.01f, "%.4f");

            // 平行光源の色設定
            ImGui::ColorEdit3("Directional Light Color", &lightCol.x);

            ImGui::Separator();
            ImGui::Text("Light Settings"); // 新しいセクションのヘッダー

            ImGui::SliderFloat("Light Intensity", &m_lightParams.lightIntensity, 0.0f, 5.0f);
            ImGui::SliderFloat("Contrast Power", &m_lightParams.contrastPower, 0.1f, 3.0f);
            ImGui::SliderFloat("PointLight Intensity", &m_lightParams.pointLightIntensity, 0.0f, 10.0f); // ←追加
            ImGui::Separator();

            ImGui::Text("OutLine Setting");
            ImGui::ColorEdit4("Color", &m_outlineColor.x);
            ImGui::SliderFloat("Thickness", &m_outlineThickness, 0.01f, 1.0f);

            ImGui::Separator();
            ImGui::Text("Bloom Settings");
            ImGui::SliderFloat("Bloom Threshold", &bloomer->bloom_extraction_threshold, 0.0f, 5.0f);
            ImGui::SliderFloat("Bloom Intensity", &bloomer->bloom_intensity, 0.0f, 5.0f);


            // シャドウマップのプレビュー表示
            ImGui::Separator();
            ImGui::Text("Shadow Map Preview:");
            ID3D11ShaderResourceView* shadowSRV = Graphics::Instance().GetShadowMapSRV();
            if (shadowSRV) {
                ImGui::Image((ImTextureID)shadowSRV, ImVec2(200, 200));
            }
        }
    }
    ImGui::End();


#endif
}

void Scene::Render(CameraBase* camera, bool isEditor)
{
    if (!scene_framebuffer || !bloomer)return;
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();
    ShapeRenderer* shapeRenderer = graphics.GetShapeRenderer();
    ModelRenderer* modelRenderer = graphics.GetModelRenderer();

    // 描画準備
    RenderContext rc;
    rc.deviceContext = graphics.GetDeviceContext();
    rc.renderState = graphics.GetRenderState();
    rc.lightDirection = lightDir;
    rc.lightColor = lightCol;
    rc.ambientColor = ambientCol;
    rc.lightParams = m_lightParams;
    rc.outlineColor.outlineColor = m_outlineColor;
    rc.outlineParams.outlineThickness = m_outlineThickness;

    DirectX::XMStoreFloat4x4(&rc.view, camera->GetView());
    DirectX::XMStoreFloat4x4(&rc.projection, camera->GetProjection());

    // ライトのVP行列計算
    {
        using namespace DirectX;
        XMVECTOR LDir = XMVector3Normalize(XMLoadFloat3(&lightDir));
        XMVECTOR LFocus = XMVectorSet(0, 0, 0, 0);
        XMVECTOR LEye = LFocus - (LDir * shadowRange);
        XMVECTOR LUp = XMVectorSet(0, 1, 0, 0);
        XMMATRIX LV = XMMatrixLookAtLH(LEye, LFocus, LUp);
        XMMATRIX LP = XMMatrixOrthographicLH(shadowRange, shadowRange, 0.1f, 200.0f);
        XMStoreFloat4x4(&lightView, LV);
        XMStoreFloat4x4(&lightProjection, LP);
        XMStoreFloat4x4(&lightVP, LV * LP);
    }
    rc.lightViewProjection = lightVP;
    rc.shadowParams = m_shadowParams;

    // shadow pass
    {
        graphics.BeginShadowMap();
        dc->PSSetShader(nullptr, nullptr, 0);

        RenderContext shadowRC = rc;
        shadowRC.view = lightView;
        shadowRC.projection = lightProjection;
        shadowRC.lightViewProjection = lightVP;
        rc.shadowParams = m_shadowParams;

        for (auto& actor : actors)
        {
            bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
            if (active)
            {
                // ShaderIdをShadowに設定して描画
                auto modelRender = actor->GetComponent<ModelRender>();
                if (modelRender)
                {
                    actor->Render(shadowRC, modelRenderer);
                }
            }
        }
        modelRenderer->SetShaderId(ShaderId::Shadow);
        modelRenderer->FlushAll(shadowRC);
        graphics.EndShadowMap();
    }
   

    // scene_framebuffer に描画
    //graphics.SetRenderTargets();
    //graphics.Clear(0.2f, 0.2f, 0.2f, 1.0f);

    scene_framebuffer->clear(dc, 0.2f, 0.2f, 0.2f, 1.0f);
    scene_framebuffer->activate(dc);

    rc.shadowMap = graphics.GetShadowMapSRV();
    rc.shadowSampler = graphics.GetShadowSampler();
    if (isEditor)
    {
        modelRenderer->DebugImGui();
    }
    

    // skymap描画
    DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.view);
    DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.projection);
    DirectX::XMFLOAT4X4 vp;
    DirectX::XMStoreFloat4x4(&vp, V * P);
    DirectX::XMFLOAT3 pos = camera->GetEye();
    sky->blit(rc, vp, { pos.x, pos.y, pos.z, 1.0f });

    dc->PSSetShaderResources(8, 1, &rc.shadowMap);
    dc->PSSetSamplers(8, 1, &rc.shadowSampler);

    // 3Dモデル描画
    for (auto& actor : actors)
    {
        bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
        if (active)
            actor->Render(rc, modelRenderer);
    }
    modelRenderer->SetShaderId(ShaderId::Lambert);
    modelRenderer->FlushAll(rc);

    // アウトライン描画
    for (auto& actor : actors)
    {
        bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
        if (active)
        {
            auto modelRender = actor->GetComponent<ModelRender>();
            if (modelRender)
            {
                actor->Render(rc, modelRenderer);   
            }
        }
    }
    modelRenderer->SetShaderId(ShaderId::Outline);
    modelRenderer->FlushAll(rc);

    // エフェクト
    EffectManager::Instance().Render(rc.view, rc.projection);

    // 3Dデバッグ描画
    if (isEditor)
    {
        for (auto& actor : actors)
        {
            bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
            if (active)
                actor->RenderDebug(rc, shapeRenderer);
        }
    }

    scene_framebuffer->deactivate(dc);

    // BLOOM
    if (bloomer)
    {
        ID3D11ShaderResourceView* null_srvs[8] = { nullptr };
        dc->PSSetShaderResources(0, 8, null_srvs);
        auto rs = graphics.GetRenderState();
        auto sp = rs->GetSamplerState(SamplerState::LinearBorderBlack);
        dc->PSSetSamplers(3, 1, &sp);
        bloomer->make(dc, scene_framebuffer->shader_resource_views[0].Get());

        dc->OMSetDepthStencilState(
            graphics.GetRenderState()->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
        dc->RSSetState(
            graphics.GetRenderState()->GetRasterizerState(RasterizerState::SolidCullNone));
        dc->OMSetBlendState(
            graphics.GetRenderState()->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);

        ID3D11ShaderResourceView* srvs[] =
        {
            scene_framebuffer->shader_resource_views[0].Get(),
            bloomer->shader_resource_view(),
        };
        bloomer->draw(dc, srvs, 2, final_pass_ps.Get());

        dc->OMSetBlendState(graphics.GetRenderState()->GetBlendState(BlendState::Transparency), nullptr, 0xFFFFFFFF);
        dc->OMSetDepthStencilState(graphics.GetRenderState()->GetDepthStencilState(DepthState::NoTestNoWrite), 0);

        // 2. ビューポートも再設定
        D3D11_VIEWPORT vp = graphics.GetViewport();
        dc->RSSetViewports(1, &vp);

        // 3. スプライトのソートと描画
        std::vector<std::pair<int, Actor*>> spriteActors;
        for (auto& actor : actors) {
            bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
            if (!active) continue;
            auto spr = actor->GetComponent<SpriteRender>();
            if (spr && spr->enabled) spriteActors.push_back({ spr->GetSortOrder(), actor.get() });
        }
        std::sort(spriteActors.begin(), spriteActors.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

        for (auto& [order, actor] : spriteActors) {
            actor->Draw(rc);
        }
    }


}

void Scene::DrawGUI()
{
	ImGuiIO& io = ImGui::GetIO();

	const float windowPadding = 10.0f;

	ImVec2 windowPos = ImVec2(
		io.DisplaySize.x - windowPadding,
		windowPadding
	);

	ImVec2 windowPivot = ImVec2(1.0f, 0.0f);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver, windowPivot);
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);


}
