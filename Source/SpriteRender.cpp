#include "SpriteRender.h"
#include "Factory.h"
#include "Actor.h"

REGISTER_COMPONENT(ComponentID::SpriteRender,SpriteRender)

void SpriteRender::Draw(RenderContext& rc)
{
    auto tran = owner->GetComponent<Transform>();
	if (spr)
	{
		spr->Render(rc, tran->GetWorldPosition().x, tran->GetWorldPosition().y, 0, 120 * tran->GetWorldScale().x, 120 * tran->GetWorldScale().y, 0, 1, 1, 1, 1);
	}
}

void SpriteRender::Update(float elapsedTime)
{
	
}

std::unique_ptr<Component> SpriteRender::Clone() const
{
	auto c = std::make_unique<SpriteRender>();

	c->SetSprite(std::make_unique<Sprite>(texturepath.c_str()));
	if (texturepath !="")
	{ 
		c->SetString(texturepath.c_str());
	}


	return c;
}

void SpriteRender::Serialize(nlohmann::json& j) const
{
	j["TexturePath"] = texturepath;
}

void SpriteRender::DrawInspector()
{
	ImGui::Checkbox("enabled", &enabled);

	if (!texturepath.empty())
	{
		std::string filename = std::filesystem::path(texturepath).filename().string();
		ImGui::Text(filename.c_str());
	}

	if (ImGui::Button("Select Sprite"))
	{
		std::string full = OpenDialog::OpenLoadFileDialog();

		if (!full.empty())
		{
			texturepath = ToDataPath(full);

			spr = std::make_unique<Sprite>(texturepath.c_str());
		}
	}
}

void SpriteRender::Deserialize(nlohmann::json& j)
{
	texturepath = j["TexturePath"];

	if (texturepath != "")
	{
		spr = std::make_unique<Sprite>(texturepath.c_str());
	}
}
