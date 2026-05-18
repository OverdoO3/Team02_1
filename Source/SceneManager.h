#pragma once
#include "Scene.h"
#include "LogManager.h"

class SceneManager
{
public:
	SceneManager() {}
	~SceneManager() {}

public:
	void Initialize();
	void Update(float elapsedTime);
	void Render(CameraBase* camera,bool isEditor);
	void DrawGUI();
	void Clear();
	void ChangeScene(std::unique_ptr<Scene> scene,const char* path = "");

	void NewScene();
	void SaveEditorScene(const std::string& path);
	void LoadEditorScene(const std::string& path);

	inline bool GetPlayState()
	{
		return playState;
	}

	inline Scene* GetCurrentScene()
	{
		return (playState)
			? runtimeScene.get()
			: editorScene.get();
	}


	inline void StopPlay()
	{
		if (runtimeScene)
		{
			runtimeScene->physics.Flush();
		}
		runtimeScene = nullptr;
		playState = false;
		currentScene = GetCurrentScene();
	}

	inline void StartPlay()
	{
		runtimeScene = editorScene->Clone();
		runtimeScene->sceneManager = this;
		runtimeScene->playState = true;
		playState = true;

		//Pause中に画像を出す用
		for (auto& actor : runtimeScene->actors)
		{
			actor->SetScene(runtimeScene.get());
		}

		editorScene->physics.DeleteAllQueue();
		currentScene = GetCurrentScene();
		currentScene->physics.DeleteAllQueue();
	}

	void SetPause(bool pause) { isPaused = pause; }
	bool IsPaused()const { return isPaused; }

	void TogglePause() { isPaused = !isPaused; }

	void RequestSceneChange(const std::string& path)
	{
		m_pendingScenePath = path;
		m_hasPendingScene = true;
	}

	std::string GetCurrentScenePath() const { return m_currentScenePath; }
	void LoadPauseUI(const std::string& path);  // ポーズUI読み込み


private:
	//編集用のシーン
	std::unique_ptr<Scene> editorScene = nullptr;
	//実行用のシーン
	std::unique_ptr<Scene> runtimeScene = nullptr;

	Scene* currentScene = nullptr;
	std::unique_ptr<Scene> nextScene = nullptr;

	bool playState = false;
	bool isPaused = false;
	bool nextSceneIsRuntime = false;
	std::string m_pendingScenePath = "";
	bool m_hasPendingScene = false;
	std::string m_currentScenePath = "";

	std::vector<std::unique_ptr<Actor>> pauseActors;
	std::string m_pauseUIPath = "Scenes/PauseUI.json";

	RenderTarget sceneRT;
	RenderTarget gameRT;
};