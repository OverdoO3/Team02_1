#pragma once
#include "Actor.h"
#include "Component.h"
#include "Prefab.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class PrefabManager
{
public:
	static PrefabManager& Instance()
	{
		static PrefabManager instance;
		return instance;
	}

	PrefabManager() = default;
	~PrefabManager() = default;

	void MakePrefab(Actor* actor);
	//Actor* Instantiate(std::string prefabName, Scene* scene);

	Actor* InstantiateFromFile(
		const std::string& path,
		Scene* scene);
private:
	std::unordered_map<std::string, std::unique_ptr<Prefab>> prefabs;
};