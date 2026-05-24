#include "flower.h"
#include "Factory.h"
#include "Actor.h"
#include <EffectRender.h>
#include "System/Audio.h"

//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::flower, flower)

void flower::OnAwake(float elapsedTime)
{
	auto temp = owner->GetComponent<ThermalBody>();
	if (!temp)return;
	if (temp->GetHeat() == -1)
	{
		owner->GetComponent<ModelRender>()->PlayAnimation("flower_idle",true);
		isOpen = true;
	}
	else
	{
		owner->GetComponent<ModelRender>()->PlayAnimation("bud_idle", true);
		isOpen = false;
	}
}

void flower::Update(float elapsedTime)
{
	auto temp = owner->GetComponent<ThermalBody>();
	if (!temp)return;
	auto boxCollider = owner->GetComponent<BoxCollider>();
	if (!boxCollider)return;
	auto model = owner->GetComponent<ModelRender>();
	if (!model)return;

	switch (temp->GetHeat())
	{
	case -1:
		isOpen = true;
		break;
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

	if (isOpen && !once)
	{
		once = true;
		model->PlayAnimation("bud_open", false);
		//‰Ô‚ªŠJ‚­‰¹
		Audio::Instance().PlaySE(ToDataPath("Data/Sound/SE_flower_open").c_str());
		auto eff = owner->GetComponent<EffectRender>();
		if (eff)
		{
			eff->Play();
		}
	}

	if (!model->GetAnimationPlaying() && isOpen && !once2)
	{
		boxCollider->size.y = saveY;
		model->SetString(openPath);
		model->PlayAnimation("flower_open", false);
		once2 = true;
	}

	if (once2&&!model->GetAnimationPlaying())
	{
		model->PlayAnimation("flower_idle",true);
	}

	if (death)
	{
		if (timer > 0)
		{
			timer -= elapsedTime;
			auto tran = owner->GetComponent<Transform>();
			tran->SetLocalScale({1,timer,1});
			model->enabled = false;
		}
	}

	if (deathY == 0)
	{
		boxCollider->enabled = false;
	}
}

void flower::DrawInspector()
{
	ImGui::InputFloat("openY", &saveY);
	ImGui::InputFloat("closeY", &deathY);
}

void flower::Serialize(nlohmann::json& j) const
{
	j["openY"] = saveY;
	j["closeY"] = deathY;
}

void flower::Deserialize(nlohmann::json& j)
{
	saveY = j["openY"];
	deathY = j["closeY"];
}

std::unique_ptr<Component> flower::Clone() const
{
	auto c = std::make_unique<flower>();
	c->saveY = saveY;
	c->deathY = deathY;
	return c;
}
