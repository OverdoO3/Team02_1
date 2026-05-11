#include <filesystem>
#include <imgui.h>
#include "System/Graphics.h"
#include "ResourceManager.h"

// モデルリソース読み込み
std::shared_ptr<ModelResource> ResourceManager::LoadModelResource(const char* filename)
{
	//読み込み済みなら既存を返す
	if (models.find(filename) != models.end())
	{
		if (models[filename].expired() == false)
		{
			return models[filename].lock();
		}
	}
	//新規モデルリソース作成＆読み込み
	auto m = std::make_shared<ModelResource>();
	m->Load(Graphics::Instance().GetDevice(), filename);

	//読み込み管理用変数に登録
	models[filename] = m;

	return m;
}

// デバッグGUI描画
void ResourceManager::DrawDebugGUI()
{
	if (ImGui::CollapsingHeader("Resource", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto it = models.begin(); it != models.end(); ++it)
		{
			std::filesystem::path filepath(it->first);

			int use_count = it->second.use_count();
			ImGui::Text("use_count = %5d : %s", use_count, filepath.filename().u8string().c_str());
		}
	}
}

