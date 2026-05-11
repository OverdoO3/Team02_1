#include "System/Graphics.h"
#include "System/Input.h"
#include "LoadingController.h"
#include "SceneManager.h"

void LoadingController::Start(std::unique_ptr<Scene> next)
{
	nextScene = std::move(next);
	thread = new std::thread(&LoadingController::LoadingThread, this);
}

void LoadingController::Initialize(SceneManager* sm)
{
	sceneManager = sm;
}

void LoadingController::Finalize()
{
	if (thread)
	{
		//スレッド終了化
		if (thread->joinable())
		{
			thread->join();
		}
		delete thread;
		thread = nullptr;
	}
}

void LoadingController::Update(float elapsedTime)
{
	//準備完了したらシーン切り替え
	if (nextScene != nullptr && nextScene->IsReady())
	{
		sceneManager->ChangeScene(std::move(nextScene));
		nextScene = nullptr;
	}
}

void LoadingController::LoadingThread()
{
	//COM関連の初期化
	CoInitialize(nullptr);
	//次のシーンの初期化
	nextScene->Initialize();
	//スレッドが終わる前に終了か
	CoUninitialize();
	//次のシーンの準備完了設定
	nextScene->SetReady();
}
