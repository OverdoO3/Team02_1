#include "SceneSerializer.h"
#include "Scene.h"
#include "Actor.h"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool SceneSerializer::Save(const Scene& scene, const std::string& path)
{
    json j;

    j["scene"]["actors"] = json::array();

    for (const auto& actor : scene.actors)
    {
        json actorJson;

        actor->Serialize(actorJson);

        j["scene"]["actors"].push_back(actorJson);
    }

    std::ofstream file(path);
    if (!file.is_open())
        return false;

    file << j.dump(4);

    return true;
}

bool SceneSerializer::Load(Scene& scene, const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    json j;
    file >> j;

    scene.actors.clear();

    if (!j.contains("scene") || !j["scene"].contains("actors"))
        return false;

    uint64_t maxID = 0;

    for (auto& elem : j["scene"]["actors"])
    {
        std::string type = elem.value("type", "Actor");

        auto actor = CreateActorByType(type);
        if (!actor) continue;

        actor->SetScene(&scene);

        actor->Deserialize(elem);

        maxID = max(maxID, actor->id);

        scene.actors.push_back(std::move(actor));
    }

    Actor::SetNextID(maxID + 1);

    for (auto& actor : scene.actors)
    {
        if (actor->parentid != -1)
        {
            Actor* parent = scene.FindActor(actor->parentid);

            if (parent)
            {
                actor->SetParent(parent);
            }
        }
    }

    return true;
}
