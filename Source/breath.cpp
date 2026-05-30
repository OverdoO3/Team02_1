#include "breath.h"
#include "Actor.h"
#include "Factory.h"

//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::breath, breath)

void breath::OnAwake(float elapsedTime)
{
}

void breath::Update(float elapsedTime)
{
	if (currentHeat != owner->GetParent()->GetComponent<ThermalBody>()->GetHeat())
	{
		if (owner->GetParent()->GetComponent<ThermalBody>()->GetHeat() == 0)
		{
			isbreath = true;
			timer = 2.0f;
		}
	}

	if (isbreath)
	{
		timer -= elapsedTime;
		if (timer < 0)
		{
			isbreath = false;
			owner->GetComponent<ThermalBody>()->SetHeat(0);
		}
	}
	currentHeat = owner->GetParent()->GetComponent<ThermalBody>()->GetHeat();
}

void breath::DrawInspector()
{
}

void breath::Serialize(nlohmann::json& j) const
{
}

void breath::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> breath::Clone() const
{
	return std::make_unique<breath>();
}
