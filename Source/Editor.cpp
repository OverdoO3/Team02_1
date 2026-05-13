#include "Editor.h"
#include "ComponentManager.h"

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
					selectedActor = nullptr;
					return;
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
				DirectX::XMFLOAT3 hitPos, hitNormal;
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
					if (auto sprRender = actor->GetComponent<SpriteRender>())
					{
						auto tran = actor->GetComponent<Transform>();
						auto worldPos = tran->GetWorldPosition();

						float dx = worldPos.x - rayOrigin.x;
						float dy = worldPos.y - rayOrigin.y;
						float dz = worldPos.z - rayOrigin.z;
						float dist = sqrtf(dx * dx + dy * dy + dz * dz);

						if (dist < 30.0f && dist < minDist)
						{
							minDist = dist;
							nowDistActor = actor.get();

							hitPos = worldPos;
						}
					}
				}
				hitp = hitPos;
				selectedActor = nowDistActor;
			}
		}
	}
}

void Editor::HandleGizmo(ImVec2 pos, ImVec2 size)
{
	EditorCamera* camera = editorCamera.get();
	if (!selectedActor) return;

	if (ImGui::IsKeyPressed('W'))
		operation = ImGuizmo::TRANSLATE;

	if (ImGui::IsKeyPressed('E'))
		operation = ImGuizmo::ROTATE;

	if (ImGui::IsKeyPressed('R'))
		operation = ImGuizmo::SCALE;

	auto transform = selectedActor->GetComponent<Transform>();

	ImGuizmo::SetOrthographic(false);

	ImGui::Separator();

	float snap[3] = {};

	if (useSnap)
	{
		if (operation == ImGuizmo::TRANSLATE)
			snap[0] = snap[1] = snap[2] = 1.0f;

		if (operation == ImGuizmo::ROTATE)
			snap[0] = snap[1] = snap[2] = 15.0f;

		if (operation == ImGuizmo::SCALE)
			snap[0] = snap[1] = snap[2] = 0.1f;
	}

	ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

	DirectX::XMMATRIX view = camera->GetView();
	DirectX::XMMATRIX proj = camera->GetProjection();

	DirectX::XMFLOAT4X4 viewF, projF;
	DirectX::XMStoreFloat4x4(&viewF, view);
	DirectX::XMStoreFloat4x4(&projF, proj);

	DirectX::XMFLOAT4X4 world = transform->GetWorldMatrix();

	ImGuizmo::Manipulate(
		&viewF._11,
		&projF._11,
		operation,
		mode,
		&world._11,
		nullptr,
		useSnap ? snap : nullptr
	);

	if (ImGuizmo::IsUsing())
	{
		DirectX::XMVECTOR s, r, t;

		DirectX::XMMatrixDecompose(
			&s, &r, &t,
			DirectX::XMLoadFloat4x4(&world)
		);

		DirectX::XMFLOAT3 scale;
		DirectX::XMFLOAT4 rotation;
		DirectX::XMFLOAT3 position;

		DirectX::XMStoreFloat3(&scale, s);
		DirectX::XMStoreFloat4(&rotation, r);
		DirectX::XMStoreFloat3(&position, t);

		transform->SetLocalPosition(position);
		transform->SetLocalScale(scale);
		transform->SetLocalRotation(rotation);
	}
}

void Editor::DrawHierarchy(Scene* scene)
{
	ImGui::Begin("Hierarchy", nullptr);

	if (ImGui::Button("NewActor"))
	{
		selectedActor = CreateActor("NewActor", scene);
	}

	for (auto& actor : scene->actors)
	{
		if (actor->GetParent() == nullptr)
		{
			DrawActorNode(actor.get());
		}
	}

	ImGui::End();
}

void Editor::DrawInspector(Scene* scene)
{
	ImGui::Begin("Inspector");
	if (selectedActor)
	{
		if (InputC::KeyDown('D') && InputC::KeyPressed(VK_CONTROL))
		{
			scene->adderActors.emplace_back(selectedActor->Clone());
		}

		ImGui::Text("Name: %s", selectedActor->name.c_str());
		ImGui::SameLine();
		ImGui::Checkbox("##enabled", &selectedActor->setActive);

		Inspector::DrawActorHeader(selectedActor);

		for (auto& comp : selectedActor->GetComponents())
		{
			if (!comp)continue;
			if (Inspector::DrawComponentHeader(comp.get(), selectedActor))
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
					selectedActor->AddComponentByID(info);
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

	// これが選択表示
	if (actor == selectedActor)
		flags |= ImGuiTreeNodeFlags_Selected;

	if (actor->setActive)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 180));
	}

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
		selectedActor = actor;
	}

	// ダブルクリックRename
	if (actor == selectedActor &&
		ImGui::IsItemHovered() &&
		ImGui::IsMouseDoubleClicked(0))
	{
		renameTarget = actor;

		strcpy_s(renameBuffer,
			actor->name.c_str());
	}

	// F2 Rename
	if (actor == selectedActor &&
		InputC::KeyDown(VK_F2))
	{
		renameTarget = actor;

		strcpy_s(renameBuffer,
			actor->name.c_str());
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

