#pragma once
#include "System/Sprite.h"
#include "Scene.h"
#include <thread>

//ÉçÅ[Éh
class LoadingController
{
public:
	LoadingController() {};
	~LoadingController() {}

	void Start(std::unique_ptr<Scene> next);

	void Initialize(SceneManager* sm);
	void Finalize();
	void Update(float elapsedTime);

	void LoadingThread();
private:
	SceneManager* sceneManager;

	float angle = 0.0f;
	std::unique_ptr<Scene> nextScene = nullptr;
	std::thread* thread = nullptr;
};