#include "Scene.h"
#include "SceneManager.h"
#include "EffectManager.h"
#include "OpenDialog.h"

void Scene::Initialize(const char* path)
{
	SceneSerializer::Load(*this,path);
	InitializeAfterLoad();
	sky = std::make_unique<sky_map>(Graphics::Instance().GetDevice(), "Data/skymap/sky_cloud.hdr",false);

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

		//アップデート
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

		adderActors.clear();
		physics.Flush();
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

}

void Scene::Render(CameraBase* camera, bool isEditor)
{
    if(!scene_framebuffer || !bloomer)return;
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
                    modelRender->SetShaderId(ShaderId::Shadow);
                    actor->Render(shadowRC, modelRenderer);
                    modelRender->SetShaderId(ShaderId::Lambert);
                }
            }
        }
        graphics.EndShadowMap();
    }

    // scene_framebuffer に描画
    //graphics.SetRenderTargets();
    //graphics.Clear(0.2f, 0.2f, 0.2f, 1.0f);

    scene_framebuffer->clear(dc, 0.2f, 0.2f, 0.2f, 1.0f);
    scene_framebuffer->activate(dc);



    rc.shadowMap = graphics.GetShadowMapSRV();
    rc.shadowSampler = graphics.GetShadowSampler();
    modelRenderer->DebugImGui();

    dc->PSSetShaderResources(8, 1, &rc.shadowMap);
    dc->PSSetSamplers(8, 1, &rc.shadowSampler);

    // skymap描画
    DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.view);
    DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.projection);
    DirectX::XMFLOAT4X4 vp;
    DirectX::XMStoreFloat4x4(&vp, V * P);
    DirectX::XMFLOAT3 pos = camera->GetEye();
    sky->blit(rc, vp, { pos.x, pos.y, pos.z, 1.0f });

    // 3Dモデル描画
    for (auto& actor : actors)
    {
        bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
        if (active)
            actor->Render(rc, modelRenderer);
    }

    // アウトライン描画
    for (auto& actor : actors)
    {
        bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
        if (active)
        {
            auto modelRender = actor->GetComponent<ModelRender>();
            if (modelRender)
            {
                modelRender->SetShaderId(ShaderId::Outline);
                actor->Render(rc, modelRenderer);
                modelRender->SetShaderId(ShaderId::Lambert);
            }
        }
    }


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
    }


    // 2Dスプライト描画
    for (auto& actor : actors)
    {
        bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
        if (active)
            actor->Draw(rc);
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
