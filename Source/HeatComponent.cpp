#include "HeatComponent.h"
#include "Actor.h"
#include "Factory.h"
#include "Scene.h"
#include "Collision.h"
#include "StateEffect.h"
#include "ThermalBody.h"

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
			toThermal->SetHeat(thermal->GetHeat());
			m_insideActors.push_back(actor.get());
		} 
	}
	m_canAbsorb = false;
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
			m_canAbsorb = true;
			break; // ˆê‚Â‚Å‚à”ÍˆÍ“à‚È‚çUIƒtƒ‰ƒOON‚ÅOK
		}
	}

	if (InputC::KeyDown(VK_LBUTTON))
	{
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
				//int stealHeat = Receiver->GetHeatNum() * -1;
				//actor->GetComponent<ThermalBody>()->SetHeat(thermal);
			}
		}
	}

	auto effectstate = owner->GetComponent<StateEffect>();
	if (!effectstate) return;  // © ‚±‚ê‚ð’Ç‰Á

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

bool HeatTransfer::IsStatusActive() const
{
	auto thermal = owner->GetComponent<ThermalBody>();
	if (!thermal) return false;

	return (thermal->GetHeat() != 0);
}