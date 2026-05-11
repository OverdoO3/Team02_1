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
		runtimeScene->playState = true;
		playState = true;
		editorScene->physics.DeleteAllQueue();
		currentScene = GetCurrentScene();
		currentScene->physics.DeleteAllQueue();
	}

private:
	//編集用のシーン
	std::unique_ptr<Scene> editorScene = nullptr;
	//実行用のシーン
	std::unique_ptr<Scene> runtimeScene = nullptr;

	Scene* currentScene = nullptr;
	std::unique_ptr<Scene> nextScene = nullptr;

	bool playState = false;

	RenderTarget sceneRT;
	RenderTarget gameRT;
};