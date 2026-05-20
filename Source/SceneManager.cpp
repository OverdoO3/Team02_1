#include "SceneManager.h"
#include "System/Graphics.h"
#include "Engine.h"
#include "SpriteRender.h"
#include "ButtonComponent.h"

void SceneManager::Initialize()
{
	ChangeScene(std::move(std::make_unique<Scene>()), "Scenes/Demo.json");
	LoadPauseUI("Scenes/pause.json");
	ChangeScene(std::move(std::make_unique<Scene>()), "Scenes/pause.json");

#ifdef _DEBUG
	int sw = GetSystemMetrics(SM_CXSCREEN);
	int sh = GetSystemMetrics(SM_CYSCREEN);

	Graphics::Instance().CreateRenderTarget(sceneRT, sw, sh);
	Graphics::Instance().CreateRenderTarget(gameRT, sw, sh);
#else
	int sw = GetSystemMetrics(SM_CXSCREEN);
	int sh = GetSystemMetrics(SM_CYSCREEN);
	Graphics::Instance().CreateRenderTarget(gameRT, sw, sh); // ★ モニター解像度
#endif

}

void SceneManager::Update(float elapsedTime)
{
	// ペンディングのシーン遷移を処理
	if (m_hasPendingScene)
	{
		m_hasPendingScene = false;
		auto newScene = std::make_unique<Scene>();
		newScene->sceneManager = this;
		newScene->Initialize(m_pendingScenePath.c_str());
		nextScene = std::move(newScene);
		nextSceneIsRuntime = playState;
	}

	if (nextScene != nullptr)
	{
		Clear();

		if (nextSceneIsRuntime)
		{
			// Play中のシーン遷移：runtimeScene を差し替える
			runtimeScene = std::move(nextScene);
			runtimeScene->sceneManager = this;
			runtimeScene->playState = true;
		}
		else
		{
			// 通常のシーン遷移：editorScene を差し替える
			editorScene = std::move(nextScene);
			editorScene->sceneManager = this;
		}

		nextScene = nullptr;
		nextSceneIsRuntime = false;
		currentScene = GetCurrentScene();
		m_currentScenePath = m_pendingScenePath;
		isPaused = false;
		int currentSW = GetSystemMetrics(SM_CXSCREEN);
		int currentSH = GetSystemMetrics(SM_CYSCREEN);


#ifdef _DEBUG
		// デバッグ時（エディタ環境）のみ sceneRT を作る
		Graphics::Instance().CreateRenderTarget(sceneRT, 1280, 720);
#endif
		// ゲーム画面（gameRT）はデバッグ・リリース問わず、常に現在の生解像度で正しく作り直す
		Graphics::Instance().CreateRenderTarget(gameRT, currentSW, currentSH);

		for (auto& actor : pauseActors)
		{
			actor->SetScene(currentScene);
		}

	}


	if (currentScene != nullptr)
	{
		if (isPaused)
		{
			//Pause中の処理をここに入れる
			for (auto& actor : pauseActors)
			{
				if (!actor->setActive) continue;
				auto* sr = actor->GetComponent<SpriteRender>();
				if (sr) sr->Update(elapsedTime);
			}
			// パス2: pauseActorsのButtonComponentクリック判定
			for (auto& actor : pauseActors)
			{
				if (!actor->setActive) continue;
				auto* btn = actor->GetComponent<ButtonComponent>();
				if (btn) btn->Update(elapsedTime);
			}
		}
		else
		{
			currentScene->Update(elapsedTime);
		}
	}
}

void SceneManager::Render(CameraBase* camera,bool isEditor)
{
	if (currentScene)
	{
		currentScene->Render(camera, isEditor);
	}

	if (isPaused)
	{
		// ソートしてから描画
		std::vector<std::pair<int, Actor*>> spriteActors;
		for (auto& actor : pauseActors)
		{
			if (!actor->setActive) continue;
			auto* sr = actor->GetComponent<SpriteRender>();
			if (sr && sr->enabled)
				spriteActors.push_back({ sr->GetSortOrder(), actor.get() });
		}
		std::sort(spriteActors.begin(), spriteActors.end(),
			[](const auto& a, const auto& b) { return a.first < b.first; });

		for (auto& [order, actor] : spriteActors)
		{
			auto* sr = actor->GetComponent<SpriteRender>();
			if (sr)
			{
				RenderContext rc;
				rc.deviceContext = Graphics::Instance().GetDeviceContext();
				rc.renderState = Graphics::Instance().GetRenderState();
				sr->Draw(rc);
			}
		}
	}
}

void SceneManager::DrawGUI()
{
	if (currentScene != nullptr)
	{
		currentScene->DrawGUI();
	}

#ifdef _DEBUG
	// デバッグ用ポーズウィンドウ
	if (isPaused)
	{
		ImGui::Begin("Pause Debug");
		ImGui::Text("Game is Paused");
		if (ImGui::Button("Resume"))
		{
			isPaused = false;
		}
		ImGui::End();
	}
#endif // _DEBUG
}

void SceneManager::Clear()
{
	if (currentScene != nullptr)
	{
		currentScene->Finalize();
		currentScene = nullptr;
	}
}

void SceneManager::ChangeScene(std::unique_ptr<Scene> scene, const char* path)
{
	// 即実行せずパスだけ記録して次フレームに回す
	m_pendingScenePath = path;
	m_hasPendingScene = true;
}

void SceneManager::NewScene()
{
	nextScene = std::make_unique<Scene>();
	nextScene->Initialize();

	// Play中なら止める
	runtimeScene.reset();
	playState = false;
}

void SceneManager::SaveEditorScene(const std::string& path)
{
	if (editorScene)
	{
		SceneSerializer::Save(*editorScene, path);
	}
}

void SceneManager::LoadEditorScene(const std::string& path)
{
	auto newScene = std::make_unique<Scene>();

	if (SceneSerializer::Load(*newScene, path))
	{
		editorScene = std::move(newScene);
		editorScene->sceneManager = this;
		editorScene->Initialize();
		// Play中なら止める
		runtimeScene.reset();
		playState = false;

		currentScene = GetCurrentScene();
	}
}

void SceneManager::LoadPauseUI(const std::string& path)
{
	pauseActors.clear();
	Scene temp;
	temp.sceneManager = this;
	SceneSerializer::Load(temp, path);

	for (auto& actor : temp.actors)
	{
		actor->SetScene(currentScene);

		// SpriteRenderに直接SceneManagerを渡す
		if (auto* sr = actor->GetComponent<SpriteRender>())
		{
			sr->SetSceneManager(this);
		}

		pauseActors.emplace_back(std::move(actor));
	}
}