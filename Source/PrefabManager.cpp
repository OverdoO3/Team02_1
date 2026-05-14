#include "PrefabManager.h"
#include "Scene.h"
#include "OpenDialog.h"
#include <fstream>

void PrefabManager::MakePrefab(Actor* actor)
{
	auto prefab = std::make_unique<Prefab>();
	prefab->id = actor->name;
    prefab->name = actor->name;

	json j;
	actor->Serialize(j);

    std::string path = OpenDialog::OpenLoadFileDialog();

    if (path.empty()) return;

    std::ofstream file(path);
    if (!file.is_open()) return;

    file << j.dump(4);

    prefab->path = path;

    prefabs[prefab->id] = std::move(prefab);
}

Actor* PrefabManager::InstantiateFromFile(const std::string& path, Scene* scene)
{
    std::ifstream file(path);
    if (!file.is_open())
        return nullptr;

    json j;
    file >> j;

    auto actor = std::make_unique<Actor>();

    actor->SetScene(scene);
    actor.get()->name = "PrefabActor";

    Actor* ptr = actor.get();
    actor->Deserialize(j);

    scene->actors.push_back(std::move(actor));

    return ptr;
}

//Actor* PrefabManager::Instantiate(std::string prefabName,Scene* scene)
//{
//    auto it = prefabs.find(prefabName);
//    if (it == prefabs.end())
//        return nullptr;
//
//    const Prefab& prefab = *it->second;
//
//    std::ifstream file(prefab.path);
//    if (!file.is_open())
//        return nullptr;
//
//    json j;
//    file >> j;
//
//    Actor* actor;
//    actor->Deserialize(j);
//
//    return actor;
//}