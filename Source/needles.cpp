#include "needles.h"
#include "Actor.h"
#include "Factory.h"
#include <EffectRender.h>
#include "System/Audio.h"

//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::needles, needles)

void needles::OnAwake(float elapsedTime)
{
}

void needles::Update(float elapsedTime)
{
	auto temp = owner->GetComponent<ThermalBody>();
	if (!temp)return;
	auto boxCollider = owner->GetComponent<BoxCollider>();
	if (!boxCollider)return;
	auto eff = owner->GetComponent<EffectRender>();
	if (!eff)return;

	switch (temp->GetHeat())
	{
	case 1:
		boxCollider->size.y = deathY;
		if (death == false)
		{
			death = true;
			eff->Play();
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
			tran->SetLocalScale({ 1,timer,1 });
		}
		else
		{
			owner->setActive = false;
		}
	}

	if (deathY == 0)
	{
		boxCollider->enabled = false;
	}
}

void needles::DrawInspector()
{
}

void needles::Serialize(nlohmann::json& j) const
{
}

void needles::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> needles::Clone() const
{
	return std::make_unique<needles>();
}
