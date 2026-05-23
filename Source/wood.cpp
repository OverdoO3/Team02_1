#include "wood.h"
#include "Actor.h"
#include "Factory.h"
#include <EffectRender.h>

//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::wood, wood)

void wood::OnAwake(float elapsedTime)
{
}

void wood::Update(float elapsedTime)
{
	auto thermal = owner->GetComponent<ThermalBody>();
	if (!thermal)return;

	auto eff = owner->GetComponent<EffectRender>();
	if (!eff)return;
	if (thermal->GetHeat() >= 1)
	{
		timer -= elapsedTime;
		if (death == false)
		{
			death = true;
			eff->Play();
		}
	}

	if (death)
	{
		owner->GetComponent<Transform>()->SetLocalScale({ 1,timer,1 });
		if (timer < 0)
		{
			owner->isDead = true;
		}
	}
}

void wood::DrawInspector()
{
}

void wood::Serialize(nlohmann::json& j) const
{
}

void wood::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> wood::Clone() const
{
	auto w = std::make_unique<wood>();
	return w;
}
