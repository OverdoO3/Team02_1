#pragma once
#include "SceneManager.h"
#include "PhysicsSystem.h"
#include "Input.h"

class Engine
{
public:
    Engine() {};
    ~Engine() {};

    void Initialize();
    void Finalize() {};
    void Update(float dt);
    void Render(CameraBase* editCam,CameraBase* gameCam);

    SceneManager& GetSceneManager() { return sceneManager; }

    RenderTarget& GetSceneRT() { return sceneRT; }
    RenderTarget& GetGameRT() { return gameRT; }

private:
    SceneManager sceneManager;

    RenderTarget sceneRT;
    RenderTarget gameRT;

private:
    void RenderScene(EditorCamera* camera);
    void RenderGame(EditorCamera* camera);

    void SetRenderTarget(RenderTarget& rt);
    void Clear(RenderTarget& rt);
};