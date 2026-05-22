#include "SceneManager.h"
#include "System/Graphics.h"
#include "Engine.h"
#include "SpriteRender.h"
#include "ButtonComponent.h"

bool SceneManager::m_coinFlags[4][3] = { false };

void SceneManager::Initialize()
{
    ChangeScene(std::move(std::make_unique<Scene>()), "Scenes/Demo.json");
    LoadPauseUI("Scenes/pause.json");
    ChangeScene(std::move(std::make_unique<Scene>()), "Scenes/choice.json");

    LoadLoadingUI("Scenes/loading.json");

#ifdef _DEBUG
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    Graphics::Instance().CreateRenderTarget(sceneRT, sw, sh);
    Graphics::Instance().CreateRenderTarget(gameRT, sw, sh);
#else
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    Graphics::Instance().CreateRenderTarget(gameRT, sw, sh);
#endif

    // 変数の初期化
    m_loadTimer = 0.0f;
}

void SceneManager::Update(float elapsedTime)
{
    // ─── ローディング演出中 ───
    if (m_isLoading)
    {
        bool isIrisOutFinished = true; // マスクが閉じきったかどうかのフラグ

        if (m_irisActor && m_irisActor->setActive)
        {
            if (auto* sr = m_irisActor->GetComponent<SpriteRender>())
            {
                if (sr->GetIrisMode() != SpriteRender::IrisMode::None)
                {
                    isIrisOutFinished = false;
                }
            }
        }

        // 【FadeOut状態】マスクが閉じきったらスレッドを立ててロード開始
        if (m_loadState == LoadState::FadeOut && isIrisOutFinished)
        {
            m_loadState = LoadState::Loading;
            m_isLoadCompleted = false;
            m_loadTimer = 0.0f;

            nextScene = std::make_unique<Scene>();
            nextScene->sceneManager = this;
            nextSceneIsRuntime = playState;

            // 古いスレッドが万が一残っていたら確実にjoinしておく
            if (m_loadThread.joinable()) m_loadThread.join();

            m_loadThread = std::thread([this]() {
                nextScene->Initialize(m_pendingScenePath.c_str());
                m_isLoadCompleted = true;
                });
        }

        // ─── バックグラウンドでシーンロード中の処理 ───
        if (m_loadState == LoadState::Loading)
        {
            m_loadTimer += elapsedTime;

            if (m_isLoadCompleted && m_loadTimer >= 2.0f) // 最低2秒待つ
            {
                if (m_loadThread.joinable()) m_loadThread.join();

                Clear();
                if (nextSceneIsRuntime) {
                    runtimeScene = std::move(nextScene);
                    runtimeScene->sceneManager = this;
                    runtimeScene->playState = true;
                }
                else {
                    editorScene = std::move(nextScene);
                    editorScene->sceneManager = this;
                }
                nextScene = nullptr;
                currentScene = GetCurrentScene();
                m_currentScenePath = m_pendingScenePath;
                isPaused = false;

                int csw = GetSystemMetrics(SM_CXSCREEN);
                int csh = GetSystemMetrics(SM_CYSCREEN);
#ifdef _DEBUG
                Graphics::Instance().CreateRenderTarget(sceneRT, 1280, 720);
#endif
                Graphics::Instance().CreateRenderTarget(gameRT, csw, csh);
                for (auto& actor : pauseActors)
                    actor->SetScene(currentScene);

                auto* modelRenderer = Graphics::Instance().GetModelRenderer();
                if (modelRenderer)
                {
                    modelRenderer->SetLightPath(m_currentScenePath);
                    modelRenderer->LoadLights(modelRenderer->GetCurrentLightPath());
                }

                // シーン切り替えが完了したので、マスクを「開く（アイリスイン）」
                m_loadState = LoadState::FadeIn;
                if (m_irisActor)
                {
                    m_irisActor->setActive = true;
                    if (auto* sr = m_irisActor->GetComponent<SpriteRender>())
                    {
                        sr->StartIrisIn();
                    }
                }
            }
        }

        // ─── 拡大（アイリスイン）演出中の処理 ───
        if (m_loadState == LoadState::FadeIn)
        {
            if (m_irisActor)
            {
                if (auto* sr = m_irisActor->GetComponent<SpriteRender>())
                {
                    if (sr->GetIrisMode() == SpriteRender::IrisMode::None)
                    {
                        m_isLoading = false;
                        m_loadState = LoadState::FadeOut;
                    }
                }
            }
            else
            {
                m_isLoading = false;
                m_loadState = LoadState::FadeOut;
            }
        }

        // loadingActors（マスクを含むロードUI）のコンポーネントを更新
        for (auto& actor : loadingActors)
        {
            if (!actor || !actor->setActive) continue;
            if (auto* sr = actor->GetComponent<SpriteRender>())
                sr->Update(elapsedTime);
        }

        // 🚨【超重要】ローディング中のフレームは、これより下の通常Update（通常シーン切り替えやcurrentScene->Update）を絶対に実行させずに終了する！
        return;
    }

    // ─── 通常の遷移リクエスト受付 ───
    if (m_hasPendingScene)
    {
        m_hasPendingScene = false;

        if (m_useLoadingForPending)
        {
            m_isLoading = true;
            m_loadState = LoadState::FadeOut; // まずは縮小モードにする
            m_isLoadCompleted = false;

            // ⭕【ここを追加】行き先のパス（StageかTitleか）で操作対象のマスクを切り替える
            std::string lowerPath = m_pendingScenePath;
            std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

            if (lowerPath.find("stage") != std::string::npos)
            {
                m_irisActor = m_gameIrisActor;  // GameMask をセット
            }
            else if (lowerPath.find("title") != std::string::npos)
            {
                m_irisActor = m_titleIrisActor; // TitleMask をセット
            }
            else if (lowerPath.find("choice") != std::string::npos)
            {
                m_irisActor = m_choiceIrisActor;
            }

            else
            {
                m_irisActor = nullptr;          // どちらでもなければマスク演出なし
            }

            // シーン遷移が始まった瞬間にマスクを「閉じる（アイリスアウト）」
            if (m_irisActor)
            {
                m_irisActor->setActive = true;
                if (auto* sr = m_irisActor->GetComponent<SpriteRender>())
                {
                    sr->StartIrisOut(); // 👈 スプライト側の演出をキック！
                }
            }
            return;
        }
        else
        {
            // ロードを挟まない即時遷移
            auto newScene = std::make_unique<Scene>();
            newScene->sceneManager = this;
            newScene->Initialize(m_pendingScenePath.c_str());
            nextScene = std::move(newScene);
            nextSceneIsRuntime = playState;
        }
    }

    if (nextScene != nullptr)
    {
        Clear();
        if (nextSceneIsRuntime) {
            runtimeScene = std::move(nextScene);
            runtimeScene->sceneManager = this;
            runtimeScene->playState = true;
        }
        else {
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
        Graphics::Instance().CreateRenderTarget(sceneRT, 1280, 720);
#endif
        Graphics::Instance().CreateRenderTarget(gameRT, currentSW, currentSH);
        for (auto& actor : pauseActors)
            actor->SetScene(currentScene);

        auto* modelRenderer = Graphics::Instance().GetModelRenderer();
        if (modelRenderer)
        {
            modelRenderer->SetLightPath(m_currentScenePath);
            modelRenderer->LoadLights(modelRenderer->GetCurrentLightPath());
        }
    }

    if (currentScene != nullptr)
    {
        if (isPaused)
        {
            for (auto& actor : pauseActors)
            {
                if (!actor->setActive) continue;
                if (auto* sr = actor->GetComponent<SpriteRender>())
                    sr->Update(elapsedTime);
            }
            for (auto& actor : pauseActors)
            {
                if (!actor->setActive) continue;
                if (auto* btn = actor->GetComponent<ButtonComponent>())
                    btn->Update(elapsedTime);
            }
        }
        else
        {
            currentScene->Update(elapsedTime);
        }
    }
}



void SceneManager::Render(CameraBase* camera, bool isEditor)
{
    if (currentScene)
        currentScene->Render(camera, isEditor);

    if (m_isLoading)
    {
        std::vector<std::pair<int, Actor*>> spriteActors;
        for (auto& actor : loadingActors)
        {
            if (!actor || !actor->setActive) continue;
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

    if (isPaused)
    {
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
        currentScene->DrawGUI();

#ifdef _DEBUG
    if (isPaused)
    {
        ImGui::Begin("Pause Debug");
        ImGui::Text("Game is Paused");
        if (ImGui::Button("Resume"))
            isPaused = false;
        ImGui::End();
    }
#endif
}

void SceneManager::Clear()
{
    if (currentScene != nullptr)
    {
        currentScene->Finalize();
        currentScene = nullptr;
    }
}

void SceneManager::ChangeScene(std::unique_ptr<Scene> scene, const char* path, bool useLoading)
{
    m_pendingScenePath = path;
    m_hasPendingScene = true;
    m_useLoadingForPending = useLoading;
}

void SceneManager::NewScene()
{
    nextScene = std::make_unique<Scene>();
    nextScene->Initialize();
    runtimeScene.reset();
    playState = false;
}

void SceneManager::SaveEditorScene(const std::string& path)
{
    if (editorScene)
        SceneSerializer::Save(*editorScene, path);

    // ⭕【修正】セーブ時も、ちゃんと環境用（_Env.json）のパスを作って保存する
    std::string envPath = path;
    size_t dotPos = envPath.find_last_of(".");
    if (dotPos != std::string::npos) {
        envPath = envPath.substr(0, dotPos);
    }
    envPath += "_Env.json";

    editorScene->SaveSettings(envPath); // ちゃんと _Env.json に保存
}

void SceneManager::LoadEditorScene(const std::string& path)
{
    auto newScene = std::make_unique<Scene>();
    newScene->sceneManager = this;

    // Initializeの中で、SceneSerializer::Load も LoadSettings(_Env.json) も全部綺麗にやってくれる！
    newScene->Initialize(path.c_str());

    editorScene = std::move(newScene);
    runtimeScene.reset();
    playState = false;
    currentScene = GetCurrentScene();

    m_currentScenePath = path;

    auto* modelRenderer = Graphics::Instance().GetModelRenderer();
    if (modelRenderer)
    {
        modelRenderer->SetLightPath(m_currentScenePath);
        modelRenderer->LoadLights(modelRenderer->GetCurrentLightPath());
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
        if (auto* sr = actor->GetComponent<SpriteRender>())
            sr->SetSceneManager(this);
        pauseActors.emplace_back(std::move(actor));
    }
}

void SceneManager::LoadLoadingUI(const std::string& path)
{
    loadingActors.clear();

    // ⭕ 2つのポインタを初期化する
    m_gameIrisActor = nullptr;
    m_titleIrisActor = nullptr;
    m_charaActor = nullptr;

    Scene temp;
    temp.sceneManager = this;
    SceneSerializer::Load(temp, path);

    for (auto& actor : temp.actors)
    {
        actor->SetScene(nullptr);
        if (auto* sr = actor->GetComponent<SpriteRender>())
            sr->SetSceneManager(this);

        // ⭕ "GameMask" という名前ならゲーム用変数に入れる
        if (std::string(actor->GetName()) == "GameMask")
            m_gameIrisActor = actor.get();

        // ⭕ "TitleMask" という名前ならタイトル用変数に入れる
        if (std::string(actor->GetName()) == "TitleMask")
            m_titleIrisActor = actor.get();

        if (std::string(actor->GetName()) == "ChoiceMask")
            m_choiceIrisActor = actor.get();


        loadingActors.emplace_back(std::move(actor));
    }
}


bool SceneManager::IsCoinCollected(int stageIdx, int coinIdx) const {
    if (stageIdx < 0 || stageIdx >= 4 || coinIdx < 0 || coinIdx >= 3) return false;
    return m_coinFlags[stageIdx][coinIdx];
}

void SceneManager::CollectCoin(int stageIdx, int coinIdx) {
    if (stageIdx < 0 || stageIdx >= 4 || coinIdx < 0 || coinIdx >= 3) return;
    m_coinFlags[stageIdx][coinIdx] = true;
}
