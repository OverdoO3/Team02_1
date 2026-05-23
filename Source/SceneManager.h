#pragma once
#include "Scene.h"
#include "LogManager.h"
#include <thread>
#include <atomic>

class SceneManager
{
public:
	SceneManager() {}
	~SceneManager() {}

public:

	void Initialize();
	void Update(float elapsedTime);
	void Render(CameraBase* camera, bool isEditor);
	void DrawGUI();
	void Clear();
	void ChangeScene(std::unique_ptr<Scene> scene, const char* path = "", bool useLoading = false);

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

	void SetPause(bool pause)
	{
		// ポーズ解除時にマウスをセンターにリセット
		if (isPaused && !pause)
		{
			HWND hwnd = GetActiveWindow();
			RECT rect;
			GetClientRect(hwnd, &rect);
			POINT center;
			center.x = rect.right / 2;
			center.y = rect.bottom / 2;
			ClientToScreen(hwnd, &center);
			SetCursorPos(center.x, center.y);
		}
		isPaused = pause;
	}
	bool IsPaused()const { return isPaused; }

	void TogglePause()
	{
		// ポーズ解除時にマウスをセンターにリセット
		if (isPaused)
		{
			HWND hwnd = GetActiveWindow();
			RECT rect;
			GetClientRect(hwnd, &rect);
			POINT center;
			center.x = rect.right / 2;
			center.y = rect.bottom / 2;
			ClientToScreen(hwnd, &center);
			SetCursorPos(center.x, center.y);
		}
		isPaused = !isPaused;
	}
	void RequestSceneChange(const std::string& path)
	{
		m_pendingScenePath = path;
		m_hasPendingScene = true;
	}

	enum class LoadState { FadeOut, Loading, FadeIn };


	std::string GetCurrentScenePath() const { return m_currentScenePath; }
	//void RequeatSceneChange(const std::string& path);
	void LoadPauseUI(const std::string& path);  // ポーズUI読み込み
	void LoadLoadingUI(const std::string& path);
	const std::string& GetPendingScenePath() const { return m_pendingScenePath; }
	LoadState GetLoadState()const { return m_loadState; }

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

	bool m_isGameFullScreen = false;

	std::vector<std::unique_ptr<Actor>> pauseActors;
	std::string m_pauseUIPath = "Scenes/PauseUI.json";



	RenderTarget sceneRT;
	RenderTarget gameRT;
	int sw;
	int sh;

	float m_transitionTimer = 0.0f;
	float m_transitionDuration = 1.0f;


	LoadState m_loadState = LoadState::FadeIn;
	bool m_isLoading = false;
	float m_maskScale = 1.0f;
	float m_maskScaleMin = 0.3f;
	float m_maskScaleMax = 1.0f;
	std::atomic<bool> m_isLoadCompleted = false;
	std::thread m_loadThread;
	bool m_useLoadingForPending = false;
	Actor* m_charaActor = nullptr;
	std::vector<std::unique_ptr<Actor>> loadingActors;
	Actor* m_irisActor = nullptr;

	Actor* m_gameIrisActor = nullptr;
	Actor* m_titleIrisActor = nullptr;
	Actor* m_choiceIrisActor = nullptr;

	float m_loadTimer = 0.0f;


	//Actor* m_irisActor = nullptr;   // くりぬきスプライト（カメレオン）
};