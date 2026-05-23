#include "Scene.h"
#include "SceneManager.h"
#include "EffectManager.h"
#include "OpenDialog.h"
#include "HeatComponent.h"
#include <algorithm>

void Scene::Initialize(const char* path)
{
	SceneSerializer::Load(*this,path);
	InitializeAfterLoad();

    if (path != nullptr)
    {
        m_sceneFilePath = path; 

        std::string fullPath = path;
        size_t lastSlash = fullPath.find_last_of("/\\");
        std::string fileName = (lastSlash == std::string::npos) ? fullPath : fullPath.substr(lastSlash + 1);

        size_t dotPos = fileName.find_last_of(".");
        if (dotPos != std::string::npos) {
            fileName = fileName.substr(0, dotPos);
        }
        std::string envPath = "Data/Lights/" + fileName + "_Env.json";

        LoadSettings(envPath);
    }

#ifndef _DEBUG
    playState = true; // Releaseでは常にプレイ状態
#endif
}

void Scene::Update(float elapsedTime)
{
#ifdef _DEBUG
    if (ImGui::Begin("Debug Settings"))
    {
        ImGui::Text("File Management");
        ImGui::Text("Current Scene: %s", m_sceneFilePath.c_str());

        // ─── ⭕ Windowsの \ と / の両方に対応して choice だけを抜き出す ───
        std::string fullPath = m_sceneFilePath;
        std::string fileName = "";

        size_t lastSlash = fullPath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            fileName = fullPath.substr(lastSlash + 1);
        }
        else {
            fileName = fullPath;
        }

        size_t dotPos = fileName.find_last_of(".");
        if (dotPos != std::string::npos) {
            fileName = fileName.substr(0, dotPos);
        }

        if (fileName.empty()) {
            fileName = "SceneSettings";
        }

        // 強制的に Data/Lights/ 内の相対パスにする
        std::string envPath = "Data/Lights/" + fileName + "_Env.json";
        // ───────────────────────────────────────────────────────────────────

        ImGui::Text("Env File: %s", envPath.c_str());

        if (ImGui::Button("Save Scene Settings"))
        {
            SaveSettings(envPath);
        }
        ImGui::SameLine();
        if (ImGui::Button("Load Scene Settings"))
        {
            LoadSettings(envPath);
        }

        ImGui::Separator();

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
            ImGui::Text("Light Settings");

            ImGui::SliderFloat("Light Intensity", &m_lightParams.lightIntensity, 0.0f, 5.0f);
            ImGui::SliderFloat("Contrast Power", &m_lightParams.contrastPower, 0.1f, 3.0f);
            ImGui::SliderFloat("PointLight Intensity", &m_lightParams.pointLightIntensity, 0.0f, 10.0f);
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

        for (auto& a : actors)
        {
            if (!a->isDead) continue;

            Actor* dead = a.get();

            // 親の children から除去
            Actor* parent = dead->GetParent();
            if (parent)
            {
                parent->RemoveChild(dead); // ★ メソッド経由で操作
                dead->SetParent(nullptr);
            }

            // 子の親参照を解消
            for (auto* child : dead->GetChildren())
            {
                child->SetParent(nullptr);
            }
            dead->ClearChildren(); // ★ これも追加
        }

        // その後に erase
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
#ifdef DEBUG
        modelRenderer->DebugImGui();
#endif // DEBUG
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

    // ─── アウトライン描画のループ ───
    HeatTransfer* playerHeatTransfer = nullptr;
    
    for (auto& actor : actors)
    {
        if (auto ht = actor->GetComponent<HeatTransfer>())
        {
            playerHeatTransfer = ht;
            break; // 見つかったらループを抜ける
        }
    }

    for (auto& actor : actors)
    {
        bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
        if (!active) continue;

        auto modelRender = actor->GetComponent<ModelRender>();
        if (modelRender)
        {
            auto heatReceiver = actor->GetComponent<HeatReceiver>();
            bool isInside = false;

            if (playerHeatTransfer && heatReceiver)
            {
                auto& insideList = playerHeatTransfer->GetInsideActors();
                if (std::find(insideList.begin(), insideList.end(), actor.get()) != insideList.end())
                {
                    isInside = true; // 範囲内に入っている
                }
            }

            if (!heatReceiver || !isInside)
            {
                actor->Render(rc, modelRenderer);
            }
        }
    }
    rc.outlineColor.outlineColor = m_outlineColor;
    rc.outlineParams.outlineThickness = m_outlineThickness;
    modelRenderer->SetShaderId(ShaderId::Outline);
    modelRenderer->FlushAll(rc); // 通常描画を確定


    for (auto& actor : actors)
    {
        bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
        if (!active) continue;

        auto modelRender = actor->GetComponent<ModelRender>();
        if (modelRender)
        {
            auto heatReceiver = actor->GetComponent<HeatReceiver>();
            bool isInside = false;

            if (playerHeatTransfer && heatReceiver)
            {
                auto& insideList = playerHeatTransfer->GetInsideActors();
                if (std::find(insideList.begin(), insideList.end(), actor.get()) != insideList.end())
                {
                    isInside = true; // 範囲内に入っている
                }
            }

            // 「HeatReceiverを持っていて」かつ「プレイヤーの範囲内」の奴だけここで描画
            if (heatReceiver && isInside)
            {
                actor->Render(rc, modelRenderer);
            }
        }
    }
    rc.outlineColor.outlineColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // オレンジ
    rc.outlineParams.outlineThickness = 0.4f; // 範囲内のときだけ太くする
    modelRenderer->SetShaderId(ShaderId::Outline);
    modelRenderer->FlushAll(rc); // 範囲内オブジェクトの描画を確定

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

        // ポイントライトのデバッグ球を追加
        ModelRenderer* modelRenderer = graphics.GetModelRenderer();
        point_lights* lights = modelRenderer->GetPointLights();
        for (int i = 0; i < 8; i++)
        {
            if (lights[i].range <= 0.0f) continue; // rangeが0なら描画しない

            DirectX::XMFLOAT3 pos = {
                lights[i].position.x,
                lights[i].position.y,
                lights[i].position.z
            };
            shapeRenderer->RenderSphere(rc, pos, 3.0f, lights[i].color);
        }

    }

    scene_framebuffer->deactivate(dc);

#ifdef _DEBUG

    if (isEditor)
    {
        modelRenderer->DebugImGui();
    }
#endif // _DEBUG
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

// ─── ⭕中身はそのまま、引数に const を追加してエラーを解消 ───
void Scene::SaveSettings(const std::string& path) const
{
    nlohmann::json j;

    // ライト・シャドウ関係
    j["SceneSettings"]["LightDir"] = { lightDir.x, lightDir.y, lightDir.z };
    j["SceneSettings"]["ShadowRange"] = shadowRange;
    j["SceneSettings"]["FollowPlayer"] = followPlayer;
    j["SceneSettings"]["LightColor"] = { lightCol.x, lightCol.y, lightCol.z, lightCol.w };
    j["SceneSettings"]["AmbientColor"] = { ambientCol.x, ambientCol.y, ambientCol.z, ambientCol.w };

    // シャドウパラメータ
    j["SceneSettings"]["ShadowAlpha"] = m_shadowParams.shadow_color;
    j["SceneSettings"]["ShadowBias"] = m_shadowParams.shadow_bias;

    // ライトインテンシティ関係
    j["SceneSettings"]["LightIntensity"] = m_lightParams.lightIntensity;
    j["SceneSettings"]["ContrastPower"] = m_lightParams.contrastPower;
    j["SceneSettings"]["PointLightIntensity"] = m_lightParams.pointLightIntensity; // ←追加したやつ！

    // アウトライン関係
    j["SceneSettings"]["OutlineColor"] = { m_outlineColor.x, m_outlineColor.y, m_outlineColor.z, m_outlineColor.w };
    j["SceneSettings"]["OutlineThickness"] = m_outlineThickness;

    // ブルーム関係
    if (bloomer)
    {
        j["SceneSettings"]["BloomThreshold"] = bloomer->bloom_extraction_threshold;
        j["SceneSettings"]["BloomIntensity"] = bloomer->bloom_intensity;
    }

    // 指定されたパスに直接ファイルを書き出す！
    std::ofstream ofs(path);
    if (ofs.is_open())
    {
        ofs << j.dump(4);
    }
}

void Scene::LoadSettings(const std::string& path)
{
    std::ifstream ifs(path);
    if (!ifs) return; // ファイルがなければ何もしない

    nlohmann::json j;
    ifs >> j;

    if (!j.contains("SceneSettings")) return;
    const auto& s = j["SceneSettings"];

    if (s.contains("LightDir")) {
        lightDir.x = s["LightDir"][0];
        lightDir.y = s["LightDir"][1];
        lightDir.z = s["LightDir"][2];
    }
    shadowRange = s.value("ShadowRange", shadowRange);
    followPlayer = s.value("FollowPlayer", followPlayer);

    if (s.contains("LightColor")) {
        lightCol.x = s["LightColor"][0];
        lightCol.y = s["LightColor"][1];
        lightCol.z = s["LightColor"][2];
        lightCol.w = s["LightColor"][3];
    }
    if (s.contains("AmbientColor")) {
        ambientCol.x = s["AmbientColor"][0];
        ambientCol.y = s["AmbientColor"][1];
        ambientCol.z = s["AmbientColor"][2];
        ambientCol.w = s["AmbientColor"][3];
    }

    m_shadowParams.shadow_color = s.value("ShadowAlpha", m_shadowParams.shadow_color);
    m_shadowParams.shadow_bias = s.value("ShadowBias", m_shadowParams.shadow_bias);

    m_lightParams.lightIntensity = s.value("LightIntensity", m_lightParams.lightIntensity);
    m_lightParams.contrastPower = s.value("ContrastPower", m_lightParams.contrastPower);
    m_lightParams.pointLightIntensity = s.value("PointLightIntensity", m_lightParams.pointLightIntensity);

    if (s.contains("OutlineColor")) {
        m_outlineColor.x = s["OutlineColor"][0];
        m_outlineColor.y = s["OutlineColor"][1];
        m_outlineColor.z = s["OutlineColor"][2];
        m_outlineColor.w = s["OutlineColor"][3];
    }
    m_outlineThickness = s.value("OutlineThickness", m_outlineThickness);

    if (bloomer)
    {
        bloomer->bloom_extraction_threshold = s.value("BloomThreshold", bloomer->bloom_extraction_threshold);
        bloomer->bloom_intensity = s.value("BloomIntensity", bloomer->bloom_intensity);
    }
}