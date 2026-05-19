#pragma once
#include <memory>
#include "LogManager.h"
#include "Actor.h"
#include "ImGuizmo.h"
#include "Camera.h"
#include "RayCast.h"
#include "ModelRender.h"
#include "Screen.h"
#include "Input.h"
#include "EditCamera.h"
#include "SceneSerializer.h"
#include "SpriteRender.h"
#include "Inspector.h"
#include "PhysicsSystem.h"
#include "sky_map.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <KeyInput.h>

#include "System/bloom.h"
#include "System/framebuffer.h"
#include "System/GpuResourceUtils.h"
#include <map>
#include <unordered_map>

using json = nlohmann::json;

class SceneManager;

// ImGui用
struct SceneDebugParams
{
	DirectX::XMFLOAT3* lightDir;        
	DirectX::XMFLOAT4* lightCol;        
	DirectX::XMFLOAT4* ambientCol;      
	float* shadowRange;                 
	bool* followPlayer;                 
	CbShadowParams* shadowParams;       
	CbLightParams* lightParams;         
	DirectX::XMFLOAT4* outlineColor;    
	float* outlineThickness;            
	bloom* bloomer;                     
};

class Scene
{
public:
	Scene() {}
	~Scene() {}

	std::unique_ptr<Scene> Clone() const
	{
		auto newScene = std::make_unique<Scene>();

		// original → copy の対応表
		std::unordered_map<Actor*, Actor*> map;

		// ① まず全部コピー作成
		for (auto& actor : actors)
		{
			auto copy = actor->Clone(true);

			map[actor.get()] = copy.get();

			// まだSceneには入れない
			newScene->actors.emplace_back(std::move(copy));
		}

		for (auto& [original, copy] : map)
		{
			if (original->GetParent())
			{
				Actor* parentCopy =
					map[original->GetParent()];

				copy->SetParent(parentCopy);
			}
		}

		newScene->InitializeAfterLoad();

		return newScene;
	}

	void Initialize(const char* path = "");

	void InitializeAfterLoad()
	{
		physics.Flush();

		nowCamera = nullptr;

		for (auto& actor : actors)
		{
			actor->SetScene(this);

			if (auto cam = actor->GetComponent<Camera>())
			{
				if (cam)
				{
					nowCamera = actor.get();
				}
			}
		}

		// 無ければ作る
		if (!nowCamera)
		{
			auto camera = std::make_unique<Actor>();
			camera->AddComponent<Camera>()->mainCam = true;

			nowCamera = camera.get();
			actors.emplace_back(std::move(camera));
		}

		sky = std::make_unique<sky_map>(Graphics::Instance().GetDevice(), "Data/skymap/sky_cloud.hdr", false);

		bloomer = std::make_unique<bloom>(
			Graphics::Instance().GetDevice(),
			static_cast<uint32_t>(Graphics::Instance().GetScreenWidth()),
			static_cast<uint32_t>(Graphics::Instance().GetScreenHeight())
		);

		scene_framebuffer = std::make_unique<framebuffer>(
			Graphics::Instance().GetDevice(),
			static_cast<uint32_t>(Graphics::Instance().GetScreenWidth()),
			static_cast<uint32_t>(Graphics::Instance().GetScreenHeight()),
			true
		);

		GpuResourceUtils::LoadPixelShader(
			Graphics::Instance().GetDevice(),
			"Data/Shader/final_pass_ps.cso",
			final_pass_ps.GetAddressOf()
		);

	}

	void Finalize() {};

	void Update(float elapsedTime);

	void Render(CameraBase* camera, bool isEditor);

	void DrawActorNode(Actor* actor);

	void DrawGUI();

	const char* GetName() const {};

	Actor* GetCamera() { return nowCamera; }

	bool IsReady()const { return ready; }

	void SetReady() { ready = true; }

	Actor* FindByTag(const int& tag)
	{
		for (auto& actor : actors)
		{
			if (actor->tag == tag)
				return actor.get();
		}
		return nullptr;
	}

	struct SceneRenderTarget
	{
		ID3D11Texture2D* texture = nullptr;
		ID3D11RenderTargetView* rtv = nullptr;
		ID3D11ShaderResourceView* srv = nullptr;
	};

	PhysicsSystem* GetPhysics() { return &physics; }

	Actor* FindActor(uint64_t id)
	{
		for (auto& actor : actors)
		{
			if (id == actor->id)
			{
				return actor.get();
			}
		}
		return nullptr;
	}
public:
	//マネージャー
	SceneManager* sceneManager;

	//当たり判定管理するやーつ
	PhysicsSystem physics;

	//現在稼働中のやつ
	std::vector<std::unique_ptr<Actor>> actors;

	//追加用のリスト
	std::vector<std::unique_ptr<Actor>> adderActors;

	//いまのGameカメラ
	Actor* nowCamera = nullptr;

	bool ready = false;

	bool playState = false;

	std::unique_ptr<sky_map> sky;

	bool isClear = false;

	//ブルーム用あんまいじらんといて
	std::unique_ptr<bloom> bloomer;
	std::unique_ptr<framebuffer> scene_framebuffer;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> final_pass_ps;


	//ライトの設定値
	DirectX::XMFLOAT3 lightDir = { 1.0f,-1.0f,1.0f };
	DirectX::XMFLOAT4 lightCol = { 1.0f,1.0f,1.0f,1.0f };
	DirectX::XMFLOAT4 ambientCol = { 0.2f,0.2f,0.2f,1.0f };


	DirectX::XMFLOAT4X4 lightView;
	DirectX::XMFLOAT4X4 lightProjection;
	DirectX::XMFLOAT4X4 lightVP;
	float shadowRange = 50.0f;
	bool followPlayer = true;
	CbShadowParams m_shadowParams = { 0.5f,0.0001f,{0,0} };
	//CbLight m_lightConstants;
	CbLightParams m_lightParams;
	DirectX::XMFLOAT4 m_outlineColor = { 0.0f,0.0f,0.0f,1.0f };
	float m_outlineThickness = 0.02f;
};

inline std::unique_ptr<Actor> CreateActorByType(const std::string& type)
{
	return std::make_unique<Actor>();
}
