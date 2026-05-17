#include "SceneManager.h"
#include "System/Graphics.h"
#include "Engine.h"

void SceneManager::Initialize()
{
	ChangeScene(std::move(std::make_unique<Scene>()), "Scenes/Demo.json");
}

void SceneManager::Update(float elapsedTime)
{

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

		Graphics::Instance().CreateRenderTarget(sceneRT, 1280, 720);
		Graphics::Instance().CreateRenderTarget(gameRT, 1280, 720);
	}


	if (currentScene != nullptr)
	{
		if (isPaused)
		{
			//Pause中の処理をここに入れる

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

void SceneManager::ChangeScene(std::unique_ptr<Scene> scene,const char* path)
{
	//一回Scene遷移のために試しで作る
	scene->sceneManager = this;

	scene->Initialize(path);
	nextScene = std::move(scene);

	nextSceneIsRuntime = playState;
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
		editorScene->Initialize();

		// Play中なら止める
		runtimeScene.reset();
		playState = false;

		currentScene = GetCurrentScene();
	}
}