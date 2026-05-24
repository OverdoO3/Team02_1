#include "Bridge.h"
#include "Factory.h"
#include "Actor.h"
#include "System/Audio.h"
//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::Bridge, Bridge)

void Bridge::OnAwake(float elapsedTime)
{
}

void Bridge::Update(float elapsedTime)
{
	auto temp = owner->GetComponent<ThermalBody>();
	if (!temp)return;
	auto boxCollider = owner->GetComponent<BoxCollider>();
	if (!boxCollider)return;
	auto model = owner->GetComponent<ModelRender>();
	if (!model)return;

	switch (temp->GetHeat())
	{
	case 1:
		boxCollider->size.y = deathY;
		if (death == false)
		{
			death = true;
			Audio::Instance().PlaySE(ToDataPath("Data/Sound/SE_game_reaction_fire.wav").c_str());
		}
		break;
	default:
		break;
	}

	if (death)
	{
		if (timer > 0)
		{
			timer -= elapsedTime;
			auto tran = owner->GetComponent<Transform>();
			tran->SetLocalScale({ timer,1,timer });
			model->enabled = false;
		}
	}

	if (deathY == 0)
	{
		//boxCollider->enabled = false;
	}
}

void Bridge::DrawInspector()
{
	ImGui::InputFloat("openY", &saveY);
	ImGui::InputFloat("deathY", &deathY);
}

void Bridge::Serialize(nlohmann::json& j) const
{
	j["openY"] = saveY;
	j["closeY"] = deathY;
}

void Bridge::Deserialize(nlohmann::json& j)
{
	saveY = j["openY"];
	deathY = j["closeY"];
}

std::unique_ptr<Component> Bridge::Clone() const
{
	auto c = std::make_unique<Bridge>();
	c->saveY = saveY;
	c->deathY = deathY;
	return c;
}
