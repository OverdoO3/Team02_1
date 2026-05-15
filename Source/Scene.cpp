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
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	ShapeRenderer* shapeRenderer = graphics.GetShapeRenderer();
	ModelRenderer* modelRenderer = graphics.GetModelRenderer();

	// 描画準備
	RenderContext rc;
	rc.deviceContext = dc;
	rc.lightDirection = { 0.0f, -1.0f, 0.0f };	// ライト方向（下方向）
	rc.renderState = graphics.GetRenderState();

	DirectX::XMStoreFloat4x4(&rc.view, camera->GetView());
	DirectX::XMStoreFloat4x4(&rc.projection, camera->GetProjection());

	//skymap描画
	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.view);
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.projection);
	DirectX::XMMATRIX VP = V * P;
	DirectX::XMFLOAT4X4 vp;
	DirectX::XMStoreFloat4x4(&vp, VP);

	DirectX::XMFLOAT3 pos = camera->GetEye();
	sky->blit(rc, vp, { pos.x,pos.y,pos.z,1.0f });

	 //3Dモデル描画
	{
		for (auto& actor : actors)
		{
			bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
			if (active)
			actor->Render(rc, modelRenderer);
		}
	}
	modelRenderer->FlushAll(rc);

	//エフェクト
	{
		EffectManager::Instance().Render(rc.view, rc.projection);
	}

	// 3Dデバッグ描画
	{
		if (isEditor == true)
		{
			for (auto& actor : actors)
			{
				bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
				if (active)
					actor->RenderDebug(rc, shapeRenderer);
			}
		}
	}

	// 2Dスプライト描画
	{
		for (auto& actor : actors)
		{
			bool active = actor->setActive && (!actor->GetParent() || actor->GetParent()->setActive);
			if (active)
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
