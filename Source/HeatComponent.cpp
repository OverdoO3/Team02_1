#include "HeatComponent.h"
#include "Actor.h"
#include "Factory.h"
#include "Scene.h"
#include "Collision.h"
#include "StateEffect.h"
#include <EffectRender.h>

REGISTER_COMPONENT(ComponentID::HeatTransfer, HeatTransfer)

void HeatTransfer::OnAwake(float elapsedTime)
{

}

void HeatTransfer::Update(float elapsedTime)
{
	auto thermal = owner->GetComponent<ThermalBody>();
	auto scene = owner->GetScene();
	if (!thermal) return;

	m_insideActors.clear();

	for (auto& actor : scene->actors)
	{
		auto toThermal = actor->GetComponent<ThermalBody>();
		if (!toThermal) continue;
		if (toThermal == thermal) continue;
		auto toTransform = actor->GetComponent<Transform>();
		auto transform = owner->GetComponent<Transform>();

		if (Collision::IntersectSphereVsSphere(transform->GetWorldPosition(),
			thermal->GetRadius(),
			toTransform->GetWorldPosition(),
			toThermal->GetRadius()))
		{
			if(thermal->GetHeat() != 0)
			toThermal->SetHeat(thermal->GetHeat());

			m_insideActors.push_back(actor.get());
		} 
	}

	
	if (InputC::KeyPressed(VK_LBUTTON))
	{
		bool success = false;
		for (auto& actor : scene->actors)
		{
			auto Receiver = actor->GetComponent<HeatReceiver>();
			if (!Receiver) continue;
			auto toTransform = actor->GetComponent<Transform>();
			auto transform = owner->GetComponent<Transform>();

			if (Collision::IntersectSphereVsSphere(transform->GetWorldPosition(),
				thermal->GetRadius(),
				toTransform->GetWorldPosition(),
				Receiver->GetRadius()))
			{
				thermal->SetHeat(Receiver->GetHeatNum());
				auto th = actor->GetComponent<ThermalBody>();
				if (th->GetHeat() == 1)
				{
					th->SetHeat(0);
					auto a = actor->GetComponent<EffectRender>();
					if (a)
					{
						a->Stop();
					}
				}
				success = true;
			}
		}
		if (!success)
		{
			thermal->SetHeat(0);
		}
	}


	auto effectstate = owner->GetComponent<StateEffect>();
	if (!effectstate) return;

	switch (thermal->GetHeat())
	{
	case 0:
		effectstate->SetState("normal");
		break;
	case 1:
		effectstate->SetState("hot");
		break;
	case -1:
		effectstate->SetState("water");
		break;
	case -2:
		effectstate->SetState("cold");
		break;
	default:
		break;
	}
}

void HeatTransfer::DrawInspector()
{

}

void HeatTransfer::Serialize(nlohmann::json& j) const
{

}

void HeatTransfer::Deserialize(nlohmann::json& j)
{

}

std::unique_ptr<Component> HeatTransfer::Clone() const
{
	return std::make_unique<HeatTransfer>();
}
