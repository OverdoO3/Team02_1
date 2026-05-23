#include "snowman.h"
#include "Factory.h"
#include "Actor.h"
#include <EffectRender.h>

//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::snowman, snowman)

void snowman::OnAwake(float elapsedTime)
{
}

void snowman::Update(float elapsedTime)
{
	auto eff = owner->GetComponent<EffectRender>();
	if (!eff)return;
	auto th = owner->GetComponent<ThermalBody>();
	if (th->GetHeat() == 1)
	{
		death = true;
		timer -= elapsedTime;
	}

	if (death)
	{
		owner->GetComponent<Transform>()->SetLocalScale({ 1,timer,1 });
		eff->Stop();
		if (timer < 0)
		{
			owner->isDead = true;
		}
	}
}

void snowman::DrawInspector()
{
}

void snowman::Serialize(nlohmann::json& j) const
{
}

void snowman::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> snowman::Clone() const
{
	return std::make_unique<snowman>();
}
