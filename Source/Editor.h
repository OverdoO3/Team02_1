#pragma once
#include "SceneManager.h"
#include "Camera.h"
#include "Engine.h"

class Editor
{
public:
	Editor() 
	{ 
		EditorCameraReset(); 
	}
	~Editor() {};

	void Initialize(Engine* eng) { engine = eng; }
	void Finalize() {};

	void EditorCameraReset() { editorCamera = std::make_unique<EditorCamera>(); }
	void DrawGUI(Scene* scene)
	{
		DrawHierarchy(scene);
		DrawInspector(scene);
	}

	void Update(float elapsedTime, Scene* scene)
	{	
		bool usingGizmo = ImGuizmo::IsUsing();
		bool overGizmo = ImGuizmo::IsOver();

		editorCamera->Update(elapsedTime, true, selectedActor);

		//補助
		if (selectedActor)
		{
			if (selectedActor->isDead)
			{
				selectedActor = nullptr;
			}
		}
	}

	void Render(Scene* scene)
	{
		BeginDockSpace();

		SetRenderTargets();
		DrawGUI(scene);
		DrawSceneWindow();
		DrawGameWindow();
	}

	void SetRenderTargets();

	CameraBase* GetEditorCamera() { return editorCamera.get(); }

private:
	Engine* engine = nullptr;

	//選択されてるアクター
	Actor* selectedActor = nullptr;

	//Gizmoのあれ
	ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE mode = ImGuizmo::LOCAL;
	bool useSnap = false;

	//rayのあれ
	DirectX::XMFLOAT3 rays, raye;
	DirectX::XMFLOAT3 hitp;

	//エディター用カメラ
	std::unique_ptr<EditorCamera> editorCamera;

	RenderTarget sceneRT;
	RenderTarget gameRT;

	bool isHovered;
private:
	Actor* CreateActor(const std::string& name, Scene* scene);

	void BeginDockSpace();

	void DrawMenuBar();
	void HandleSelection(Scene* scene,ImVec2 pos,ImVec2 size);
	void HandleGizmo(ImVec2 pos,ImVec2 size);

	void DrawHierarchy(Scene* scene);
	void DrawInspector(Scene* scene);

	void DrawSceneWindow();
	void DrawGameWindow();

	void DrawActorNode(Actor* actor);
};