#include "Editor.h"
#include "ComponentManager.h"
#include "PrefabManager.h"

Actor* Editor::CreateActor(const std::string& name,Scene* scene)
{
	auto actor = std::make_unique<Actor>();

	actor->SetScene(scene);
	actor.get()->name = name;

	Actor* ptr = actor.get();
	scene->actors.push_back(std::move(actor));

	return ptr;
}

void Editor::BeginDockSpace()
{
	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoDocking;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	window_flags |= ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGui::Begin("DockSpaceWindow", nullptr, window_flags);

	ImGui::PopStyleVar(2);

	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

	DrawMenuBar();

	ImGui::End();
}

void Editor::DrawMenuBar()
{
	auto& sceneManager = engine->GetSceneManager();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{
				std::string path = OpenDialog::OpenLoadFileDialog();
				if (!path.empty())
				{
					sceneManager.SaveEditorScene(path);
				}
			}
			if (ImGui::MenuItem("New"))
			{
				sceneManager.NewScene();
				ImGui::EndMenu();
				ImGui::EndMainMenuBar();
				return;
			}
			if (ImGui::MenuItem("Open"))
			{
				std::string path = OpenDialog::OpenLoadFileDialog();
				if (!path.empty())
				{
					sceneManager.LoadEditorScene(path);
					ImGui::EndMenu();
					ImGui::EndMainMenuBar();
					selectedActors.clear();
					return;
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Prefab"))
		{
			if (ImGui::MenuItem("Make Prefab"))
			{
				if (selectedActors.front())
				{
					PrefabManager::Instance().MakePrefab(selectedActors.front());
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void Editor::HandleSelection(Scene* scene, ImVec2 pos, ImVec2 size)
{
	EditorCamera* camera = editorCamera.get();
	ImGuiIO& io = ImGui::GetIO();
	{
		if (ImGui::IsItemHovered() &&
			!ImGuizmo::IsUsing() &&
			!ImGuizmo::IsOver() &&
			KeyInput::Instance().GetKeyDown(VK_LBUTTON))
		{
			DirectX::XMFLOAT3 rayOrigin, rayDir;
			if (Hit::CreateRayFromImGui(camera, pos,
				size, rayOrigin, rayDir))
			{
				DirectX::XMFLOAT3 rayEnd = {
					rayOrigin.x + rayDir.x * 1000.0f,
					rayOrigin.y + rayDir.y * 1000.0f,
					rayOrigin.z + rayDir.z * 1000.0f
				};

				float minDist = FLT_MAX;
				Actor* nowDistActor = nullptr;
				DirectX::XMFLOAT3 hitPos {};
				DirectX::XMFLOAT3 hitNormal{};
				for (auto& actor : scene->actors)
				{
					if (actor->GetComponent<ModelRender>() != nullptr)
					{
						if (actor->GetComponent<ModelRender>()->GetModel() != nullptr)
						{
							rays = rayOrigin;
							raye = rayEnd;
							if (Hit::RayCast(
								rayOrigin,
								rayEnd,
								actor->GetComponent<Transform>()->GetWorldMatrix(),
								actor->GetComponent<ModelRender>()->GetModel(),
								hitPos,
								hitNormal))
							{
								float dx = hitPos.x - camera->GetEye().x;
								float dy = hitPos.y - camera->GetEye().y;
								float dz = hitPos.z - camera->GetEye().z;
								
								float dist = sqrtf(dx * dx + dy * dy + dz * dz);
								if (dist < minDist)
								{
									minDist = dist;
									nowDistActor = actor.get();
								}
							}
						}
					}
				}
				hitp = hitPos;
				if (nowDistActor)
				{
					if (InputC::KeyDown(VK_CONTROL))
					{
						selectedActors.push_back(nowDistActor);
						anchorActor = nowDistActor;
					}
					else
					{
						selectedActors.clear();
						selectedActors.push_back(nowDistActor);
						anchorActor = nowDistActor;
					}
				}
			}
			else
			{
				selectedActors.clear();
				anchorActor = nullptr;
			}
		}
	}
}

void Editor::HandleGizmo(ImVec2 pos, ImVec2 size)
{
	if (selectedActors.empty() || anchorActor == nullptr)
		return;

	if (InputC::KeyDown(VK_DELETE))
	{
		for (auto& act : selectedActors)
		{
			act->SetParent(nullptr);
			act->isDead = true;
			anchorActor = nullptr;
			selectedActors.clear();
		}
		return;
	}

	EditorCamera* camera = editorCamera.get();

	Actor* anchor = anchorActor;
	auto* anchorTransform = anchor->GetComponent<Transform>();

	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

	DirectX::XMMATRIX view = camera->GetView();
	DirectX::XMMATRIX proj = camera->GetProjection();

	DirectX::XMFLOAT4X4 viewF, projF;
	DirectX::XMStoreFloat4x4(&viewF, view);
	DirectX::XMStoreFloat4x4(&projF, proj);

	float snap[3] = {};

	if (InputC::KeyPressed('W'))operation = ImGuizmo::TRANSLATE;
	if (InputC::KeyPressed('E'))operation = ImGuizmo::ROTATE;
	if (InputC::KeyPressed('R'))operation = ImGuizmo::SCALE;

	if (InputC::KeyDown(VK_SHIFT))
	{
		useSnap = true;
	}

	if (useSnap)
	{
		if (operation == ImGuizmo::TRANSLATE)
			snap[0] = snap[1] = snap[2] = 1.0f;

		if (operation == ImGuizmo::ROTATE)
			snap[0] = snap[1] = snap[2] = 15.0f;

		if (operation == ImGuizmo::SCALE)
			snap[0] = snap[1] = snap[2] = 0.1f;
	}

	//anchorの元ワールド
	DirectX::XMFLOAT4X4 oldWorld = anchorTransform->GetWorldMatrix();
	DirectX::XMFLOAT4X4 newWorld = oldWorld;

	// Guizmo操作
	ImGuizmo::Manipulate(
		&viewF._11,
		&projF._11,
		operation,
		mode,
		&newWorld._11,
		nullptr,
		useSnap ? snap : nullptr
	);

	if (ImGuizmo::IsUsing())
	{
		// =========================
		// ① delta計算
		// =========================
		DirectX::XMMATRIX oldM = XMLoadFloat4x4(&oldWorld);
		DirectX::XMMATRIX newM = XMLoadFloat4x4(&newWorld);

		DirectX::XMMATRIX delta =
			newM * XMMatrixInverse(nullptr, oldM);

		// =========================
		// ② 全選択に適用
		// =========================
		for (auto* actor : selectedActors)
		{
			auto* t = actor->GetComponent<Transform>();

			DirectX::XMMATRIX current =
				XMLoadFloat4x4(&t->GetWorldMatrix());

			DirectX::XMMATRIX result =
				delta * current;

			// =========================
			// ③ world → localへ戻す
			// =========================
			Actor* parent = actor->GetParent();

			DirectX::XMMATRIX parentWorld =
				parent
				? XMLoadFloat4x4(&parent->GetComponent<Transform>()->GetWorldMatrix())
				: DirectX::XMMatrixIdentity();

			DirectX::XMMATRIX local =
				result * XMMatrixInverse(nullptr, parentWorld);

			DirectX::XMVECTOR s, r, t2;
			XMMatrixDecompose(&s, &r, &t2, local);

			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT4 rot;
			DirectX::XMFLOAT3 scale;

			XMStoreFloat3(&pos, t2);
			XMStoreFloat4(&rot, r);
			XMStoreFloat3(&scale, s);

			t->SetLocalPosition(pos);
			t->SetLocalRotation(rot);
			t->SetLocalScale(scale);
		}
	}
}

void Editor::DrawHierarchy(Scene* scene)
{
	ImGui::Begin("Hierarchy");

	ImGui::SameLine();
	if (ImGui::Button("LoadPrefab"))
	{
		std::string path = OpenDialog::OpenLoadFileDialog();
		if (path.empty())return;
		PrefabManager::Instance().InstantiateFromFile(path, scene);
	}

	ImGui::SameLine();
	if (ImGui::Button("NewActor"))
	{
		CreateActor("NewActor",scene);
	}

	ImGui::BeginChild("HierarchyContent", ImVec2(0, 0), true);

	for (auto& actor : scene->actors)
	{
		if (actor->GetParent() == nullptr)
			DrawActorNode(actor.get());
	}

	// ツリーの一番下に空白のドロップ領域を作る
	ImGui::Dummy(ImGui::GetContentRegionAvail());

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload =
			ImGui::AcceptDragDropPayload("HIERARCHY_ACTOR"))
		{
			Actor* dropped = *(Actor**)payload->Data;
			dropped->SetParent(nullptr);
		}

		ImGui::EndDragDropTarget();
	}

	ImGui::EndChild();
	ImGui::End();
}

void Editor::DrawInspector(Scene* scene)
{
	ImGui::Begin("Inspector");
	if (!selectedActors.empty()&&anchorActor != nullptr)
	{
		std::vector<Actor*> newSelection;
		if (InputC::KeyPressed('D') && InputC::KeyDown(VK_CONTROL))
		{
			for (auto* a : selectedActors)
			{
				auto clone = a->Clone();
				Actor* ptr = clone.get();
				// 少しずらすと気持ちいい
				auto t = clone->GetComponent<Transform>();
				auto pos = t->GetLocalPosition();
				pos.x += 10.0f;
				t->SetLocalPosition(pos);
				clone->SetParent(a->GetParent());
				scene->actors.emplace_back(std::move(clone));
				
				newSelection.push_back(ptr);
				anchorActor = ptr;
			}
			selectedActors = newSelection;
		}

		ImGui::Text("Name: %s", anchorActor->name.c_str());
		ImGui::SameLine();
		ImGui::Checkbox("##enabled", &anchorActor->setActive);

		Inspector::DrawActorHeader(anchorActor);

		for (auto& comp : anchorActor->GetComponents())
		{
			if (!comp)continue;
			if (Inspector::DrawComponentHeader(comp.get(), anchorActor))
			{
				comp->isDeleted = true;
			}
			comp->DrawInspector();
		}

		if (ImGui::BeginCombo("Add Component", "Select"))
		{
			for (auto& info : ComponentFactory::GetRegistered())
			{
				if (ImGui::Selectable(ComponentFactory::GetName(info)))
				{
					anchorActor->AddComponentByID(info);
				}
			}

			ImGui::EndCombo();
		}
	}
	ImGui::End();
}

void Editor::DrawSceneWindow()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

	ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 size = ImGui::GetContentRegionAvail();

	ImGui::Image((ImTextureID)sceneRT.srv.Get(), size);

	Scene* scene = engine->GetSceneManager().GetCurrentScene();
	HandleSelection(scene, pos, size);

	isHovered = ImGui::IsItemHovered();

	ImGuizmo::BeginFrame();
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

	HandleGizmo(pos, size);

	ImGui::End();

	ImGui::PopStyleVar(2);
}

void Editor::DrawGameWindow()
{
	ImGui::Begin("Game");

	auto& sm = engine->GetSceneManager();

	// --- ボタン ---
	if (!sm.GetPlayState())
	{
		if (ImGui::Button("|> Play")||InputC::KeyPressed('P')&&InputC::KeyDown(VK_LCONTROL))
		{
			sm.StartPlay();
		}
	}
	else
	{
		if (ImGui::Button("|| Stop") || InputC::KeyPressed('P') && InputC::KeyDown(VK_LCONTROL))
		{
			sm.StopPlay();
		}
	}

	// 少し見やすく
	ImGui::SameLine();
	ImGui::Text(sm.GetPlayState() ? "Running" : "Stopped");

	ImVec2 size = ImGui::GetContentRegionAvail();
	// --- ゲーム画面 ---
	ImGui::Image((ImTextureID)gameRT.srv.Get(), size);
	ImGui::End();
}

void Editor::SetRenderTargets()
{
	this->sceneRT = engine->GetSceneRT();
	this->gameRT = engine->GetGameRT();
}

void Editor::DrawActorNode(Actor* actor)
{
	ImGuiTreeNodeFlags flags =
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanAvailWidth;

	static Actor* renameTarget = nullptr;
	static char renameBuffer[256] = "";

	if (renameTarget == actor)
	{
		// ラベル無しTreeNode
		bool opened = ImGui::TreeNodeEx(
			(void*)actor,
			flags,
			""
		);

		ImGui::SameLine();

		ImGui::SetNextItemWidth(150);

		if (ImGui::InputText(
			"##Rename",
			renameBuffer,
			sizeof(renameBuffer),
			ImGuiInputTextFlags_EnterReturnsTrue |
			ImGuiInputTextFlags_AutoSelectAll))
		{
			actor->name = renameBuffer;
			renameTarget = nullptr;
		}

		// Escapeキャンセル
		if (InputC::KeyDown(VK_ESCAPE))
		{
			renameTarget = nullptr;
		}

		// フォーカス外れたら確定
		if (!ImGui::IsItemActive() &&
			ImGui::IsMouseClicked(0))
		{
			actor->name = renameBuffer;
			renameTarget = nullptr;
		}

		if (opened)
		{
			for (auto child : actor->GetChildren())
			{
				DrawActorNode(child);
			}

			ImGui::TreePop();
		}

		return;
	}

	if (!selectedActors.empty())
	{
		// これが選択表示
		if (actor == selectedActors.front())
			flags |= ImGuiTreeNodeFlags_Selected;
	}

	DirectX::XMFLOAT4 color;

	if (actor->setActive)
	{
		color = { 255,255,255,255 };
	}
	else
	{
		color = { 180,180,180,180, };
	}

	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(color.x,color.y,color.z,color.w));

	bool opened = ImGui::TreeNodeEx(
		(void*)actor,
		flags,
		actor->name.c_str()
	);

	if (ImGui::BeginDragDropSource())
	{
		Actor* ptr = actor;

		ImGui::SetDragDropPayload(
			"HIERARCHY_ACTOR",
			&ptr,
			sizeof(Actor*)
		);

		ImGui::Text(actor->name.c_str());

		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload =
			ImGui::AcceptDragDropPayload("HIERARCHY_ACTOR"))
		{
			Actor* dropped =
				*(Actor**)payload->Data;

			// 自分自身はNG
			if (dropped != actor)
			{
				dropped->SetParent(actor);
			}
		}

		ImGui::EndDragDropTarget();
	}

	// クリック判定
	if (ImGui::IsItemClicked())
	{
		//if (selectedActors.empty())return;
		if (InputC::KeyDown(VK_LCONTROL))
		{
			selectedActors.emplace_back(actor);
			anchorActor = actor;
		}
		else
		{
			selectedActors.clear();
			selectedActors.emplace_back(actor);
			anchorActor = actor;
		}

	}

	if (!selectedActors.empty())
	{
		// ダブルクリックRename
		if (actor == selectedActors.front() &&
			ImGui::IsItemHovered() &&
			ImGui::IsMouseDoubleClicked(0))
		{
			renameTarget = actor;

			strcpy_s(renameBuffer,
				actor->name.c_str());
		}
	}

	if (!selectedActors.empty())
	{
		// F2 Rename
		if (actor == selectedActors.front() &&
			InputC::KeyDown(VK_F2))
		{
			renameTarget = actor;

			strcpy_s(renameBuffer,
				actor->name.c_str());
		}
	}

	if (opened)
	{
		for (auto child : actor->GetChildren())
		{
			DrawActorNode(child);
		}

		ImGui::TreePop();
	}

	ImGui::PopStyleColor();
}

