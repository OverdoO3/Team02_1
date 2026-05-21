#pragma once
#include "DirectXMath.h"
#include "Component.h"
#include <memory>
#include "System/Model.h"
#include "System/Sprite.h"
#include "LogManager.h"
#include "Transform.h"
#include "SpriteRender.h"
#include "ModelRender.h"
#include "Move.h"
#include "BoxCollider.h"
#include "ComponentManager.h"
#include "GroundCheckComponent.h"
#include "ThermalBody.h"
#include "HeatReceiver.h"

#include <nlohmann/json.hpp>
#include <Factory.h>
using json = nlohmann::json;

class Scene;

class Actor
{
public:
	//Actor自体のオンオフ
	bool setActive = true;
	bool isDead = false;
	std::string name = "Actor";
	std::string type = "Actor";	
	int tag = 0;
	uint64_t id = 0;
	uint64_t parentid = -1;

	Transform* transform;

	Actor();
	~Actor();

	void OnAwake(float elapsedTime);

	void Update(float elapsedTime);
	void UpdateWithOutPlayed(float elapsedTime);

	void Draw(RenderContext& rc);
	void Render(RenderContext& rc, ModelRenderer* renderer);
	void RenderDebug(RenderContext& rc, ShapeRenderer* renderer);

	std::unique_ptr<Actor> Clone(bool play = false) const;

	void SetScene(Scene* s);
	Scene* GetScene() { return scene; }
	std::string GetName()const { return name; }

	uint64_t GetID() const { return id; }

	bool IsDescendantOf(Actor* potentialParent);

	template<class T>
	T* GetComponent()
	{
		auto& ptr = components[(int)T::StaticID];
		return ptr ? static_cast<T*>(ptr.get()) : nullptr;
	}

	void RemoveComponent(ComponentID id)
	{
		auto& comp = components[(int)id];
		if (!comp) return;

		if (auto* col = dynamic_cast<Collider*>(comp.get()))
		{
			UnRegisterComp(col);
		}

		comp.reset(); // 実体削除
	}

	void RegisterComp(Component* comp);
	void UnRegisterComp(Component* comp);

	const std::array<std::unique_ptr<Component>,(size_t)ComponentID::COUNT>& GetComponents()const { return components; }

	//何かよくわからんけど引数が可変にできるよっていうことらしい
	template<class T, class... Args>
	inline T* AddComponent(Args && ...args)
	{
		ComponentID id = T::StaticID;

		auto comp = std::make_unique<T>(std::forward<Args>(args)...);
		comp->SetOwner(this);

		// ログ用
		std::string na = std::string(ComponentRegistry::IDToString(id)) + " to " + name;

		LogManager::Instance().AddLog(
			LogCategory::component,
			LogEvent::Add,
			na
		);

		T* ptr = comp.get();

		RegisterComp(ptr); // ←分離

		components[(int)id] = std::move(comp);

		return ptr;
	}

	//Factory用のやつ
	void AddComponent(std::unique_ptr<Component> comp);

	Component* AddComponentByID(ComponentID id);

	void SetParent(Actor* newParent)
	{
		if (newParent && newParent->IsDescendantOf(this))
			return;
		if (newParent == parent)
			return;

		if (parent)
		{
			auto& siblings = parent->children;
			siblings.erase(
				std::remove(siblings.begin(), siblings.end(), this),
				siblings.end()
			);
		}

		parent = newParent;

		if (parent)
		{
			parent->children.push_back(this);
			transform->SetParent(
				parent->GetComponent<Transform>());
		}
		else
		{
			transform->SetParent(nullptr);
		}
	}

	Actor* GetParent() const { return parent; }
	const std::vector<Actor*>& GetChildren() const { return children; }

	void RemoveChild(Actor* child)
	{
		auto it = std::find(children.begin(), children.end(), child);
		if (it != children.end())
		{
			children.erase(it);
		}
	}

	void ClearChildren()
	{
		children.clear();
	}

	void Serialize(nlohmann::json& j)const 
	{
		j["type"] = type;
		j["name"] = name;
		j["tag"] = tag;
		j["id"] = id;
		j["parent"] = parent ? parent->GetID() : -1;

		if (parent)
		{
			j["parent"] = parent->id;
		}
		else
		{
			j["parent"] = -1;
		}
		
		j["components"] = nlohmann::json::array();

		for (auto& comp : components)
		{
			if (!comp) continue;
			json compJson;
			
			ComponentID id = comp->GetID();
			const char* str = ComponentRegistry::IDToString(id);
			if (!str) continue;

			compJson["type"] = str;

			json data;
			comp->Serialize(data);

			compJson["data"] = data;

			j["components"].push_back(compJson);
		}
	}

	void Deserialize(nlohmann::json& j)
	{
		if (!j.contains("components")) return;

		name = j["name"];
		tag = j["tag"];
		id = j["id"];
		parentid = j["parent"];

		for (auto& compJson : j["components"])
		{
			std::string type = compJson["type"];
			ComponentID id;

			// string → ID
			try
			{
				id = ComponentRegistry::StringToID(type);
			}
			catch (...)
			{
				continue; // 不明コンポーネントはスキップ
			}

			Component* comp = nullptr;

			if (id == ComponentID::Transform)
			{
				comp = transform; // 既存
			}
			else
			{
				// Factoryで生成
				auto newComp = ComponentFactory::Create(id);
				if (!newComp) continue;

				comp = newComp.get();
				AddComponent(std::move(newComp));
			}

			if (comp)
			{
				comp->Deserialize(compJson["data"]);
			}
		}
	}

	void DrawActorHeader(Actor* actor)
	{
		ImGui::Text("%s", actor->name.c_str());

		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 30);

		if (ImGui::Button("...##Actor"))
		{
			ImGui::OpenPopup("ActorMenu");
		}

		if (ImGui::BeginPopup("ActorMenu"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				actor->isDead = true; 
			}

			ImGui::EndPopup();
		}
	}

	static void SetNextID(uint64_t id)
	{
		nextID = id;
	}
private:
	std::array<std::unique_ptr<Component>,(size_t)ComponentID::COUNT> components;

	Actor* parent = nullptr;
	std::vector<Actor*> children;

	Scene* scene;
	inline static uint64_t nextID = 1;
};