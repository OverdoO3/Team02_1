#include <memory>
#include <sstream>
#include <imgui.h>

#include "Framework.h"
#include "System/Input.h"
#include "System/Graphics.h"
#include "System/ImGuiRenderer.h"
#include "SceneManager.h"
#include "EffectManager.h"
#include "System/Audio.h"

#include "KeyInput.h"
#include "LogManager.h"

// 垂直同期間隔設定
static const int syncInterval = 1;

//static SceneGame sceneGame;

// コンストラクタ
Framework::Framework(HWND hWnd)
	: hWnd(hWnd)
{
	// インプット初期化
	Input::Instance().Initialize(hWnd);

	// グラフィックス初期化
	Graphics::Instance().Initialize(hWnd);

	// IMGUI初期化
	ImGuiRenderer::Initialize(hWnd, Graphics::Instance().GetDevice(), Graphics::Instance().GetDeviceContext());

	//エフェクトマネージャー初期化
	EffectManager::Instance().Initialize();

	//オーディオ
	Audio::Instance().Initialize();

	//ログマネージャ初期化
	LogManager::Instance().Initialize();

	//起動ログ
	LogManager::Instance().AddLog(LogCategory::system,LogEvent::Initialize,"GameStart");

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// シーン初期化
	//SceneManager::Instance().ChangeScene(std::move(std::make_unique<Scene>()),"Scenes/scene.json");

	//マウスカーソル初期化
	mouseCursor = std::make_unique<MouseCursor>();
	mouseCursor->Initialize("Data/Sprite/MouseCursor.png");

	engine.Initialize();
	editor.Initialize(&engine);
}

// デストラクタ
Framework::~Framework()
{
	// シーン終了化
	//sceneGame.Finalize();
	//SceneManager::Instance().Clear();

	engine.Finalize();
	editor.Finalize();

	// IMGUI終了化
	ImGuiRenderer::Finalize();
	
	//エフェクトマネージャー終了化
	EffectManager::Instance().Finalize();

	//終了ログ
	LogManager::Instance().AddLog(LogCategory::system,LogEvent::Finalize, "GameEnd");

	Audio::Instance().Finalize();

	mouseGuard = std::make_unique<MouseGuard>();

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D11Debug> d3dDebug;
	Graphics::Instance().GetDevice()->QueryInterface(
		__uuidof(ID3D11Debug),
		reinterpret_cast<void**>(d3dDebug.GetAddressOf())
	);
	if (d3dDebug)
	{
		d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}
#endif
}

// 更新処理
void Framework::Update(float elapsedTime)
{
	// インプットの生更新とImGuiは常に動かす
	Input::Instance().Update();
	ImGuiRenderer::NewFrame();

	//ポーズ中か確認用
	bool isPaused = engine.GetSceneManager().IsPaused();

	LogManager::Instance().DrawLogWindow();

	// ポーズ中でない場合のみゲームを更新
	if (!isPaused)
	{
		// キー入力の状態を確定
		KeyInput::Instance().Update();

		// パーティクルなどの更新
		EffectManager::Instance().Update(elapsedTime);

		// シーン・アクター・物理の更新
		engine.Update(elapsedTime);
	}
	else
	{
		engine.GetSceneManager().Update(elapsedTime);
	}

	if (mouseCursor)
	{
		mouseCursor->Update(hWnd);
	}

	// エディタはポーズ中も動かせるように外に出す
	editor.Update(elapsedTime, engine.GetSceneManager().GetCurrentScene());

}


// 描画処理
void Framework::Render(float elapsedTime)
{
	std::lock_guard <std::recursive_mutex> lock(Graphics::Instance().GetMutex());

	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = Graphics::Instance().GetRenderState();

	// 画面クリア
	Graphics::Instance().Clear(0, 0, 1, 1);

	// レンダーターゲット設定
	Graphics::Instance().SetRenderTargets();

	// シーン描画処理
	CameraBase* editorCam = editor.GetEditorCamera();
	CameraBase* gameCam = engine.GetSceneManager().GetCurrentScene()->GetCamera()->GetComponent<Camera>();
	engine.Render(editorCam,gameCam);

	// シーンGUI描画処理
	Scene* scene = engine.GetSceneManager().GetCurrentScene();
	editor.Render(scene);
	//SceneManager::Instance().DrawGUI();
#if 0
	// IMGUIデモウインドウ描画（IMGUI機能テスト用）
	ImGui::ShowDemoWindow();
#endif

	auto& graphics = Graphics::Instance();
	ID3D11RenderTargetView* rtv = graphics.GetBackBufferRTV();

	dc->OMSetRenderTargets(1, &rtv, graphics.GetDepthStencilView());

	ImGui::Render();
	// IMGUI描画
	ImGuiRenderer::Render(dc);


	if (mouseCursor)
	{
		mouseCursor->Draw(rc);
	}
	// 画面表示
	Graphics::Instance().Present(syncInterval);
}

// フレームレート計算
void Framework::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.
	static int frames = 0;
	static float time_tlapsed = 0.0f;

	frames++;

	// Compute averages over one second period.
	if ((timer.TimeStamp() - time_tlapsed) >= 1.0f)
	{
		float fps = static_cast<float>(frames); // fps = frameCnt / 1
		float mspf = 1000.0f / fps;
		std::ostringstream outs;
		outs.precision(6);
		outs << "FPS : " << fps << " / " << "Frame Time : " << mspf << " (ms)";
		SetWindowTextA(hWnd, outs.str().c_str());

		// Reset for next average.
		frames = 0;
		time_tlapsed += 1.0f;
	}
}

// アプリケーションループ
int Framework::Run()
{
	MSG msg = {};

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			timer.Tick();
			CalculateFrameStats();

			float elapsedTime = timer.TimeInterval();

			Update(elapsedTime);
			Render(elapsedTime);
		}
	}
	return static_cast<int>(msg.wParam);
}

// メッセージハンドラ
LRESULT CALLBACK Framework::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGuiRenderer::HandleMessage(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			auto& sm = engine.GetSceneManager();
			//std::string path = sm.GetCurrentScenePath();

			//// "stage" を含むシーンだけポーズ可能
			//if (path.find("stage") != std::string::npos)
			//{
				sm.TogglePause();
			//}
		}
		break;
	case WM_ENTERSIZEMOVE:
		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
		timer.Stop();
		break;
	case WM_EXITSIZEMOVE:
		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
		timer.Start();
		break;

	//case WM_CLOSE: 
	//	while (ShowCursor(TRUE) < 0);

	//	DestroyWindow(hWnd);
	//	break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}
