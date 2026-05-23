#include "water.h"
#include "Actor.h"
#include "Factory.h"
#include "StateEffect.h"
#include <EffectRender.h>
//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::water, water)

void water::OnAwake(float elapsedTime)
{
	auto eff = owner->GetComponent<EffectRender>();
	if (!eff)return;

	eff->SetEffect(EffectManager::Instance().LoadEffect("./Data/Effects/ice_box.efk"));
}

void water::Update(float elapsedTime)
{
	auto temp = owner->GetComponent<ThermalBody>();
	if (!temp)return;
	auto modelrender = owner->GetComponent<ModelRender>();
	if (!modelrender)return;
	auto eff = owner->GetComponent<EffectRender>();
	if (!eff)return;

	auto col = owner->GetComponent<BoxCollider>();

	if (temp->GetHeat() == currentTemp) return;
	std::unique_ptr<Model> model;
	switch (temp->GetHeat())
	{
	case -2:
 		model = std::make_unique<Model>(icepath.c_str());
		col->size.y = 10;
		modelrender->SetModel(std::move(model));
		eff->Play();
		break;
	case 1:
		model = std::make_unique<Model>(waterpath.c_str());
		col->size.y = 50;
		modelrender->SetModel(std::move(model));
		eff->Stop();
		break;
	default:
		break;
	}

	auto thermal = owner->GetComponent<ThermalBody>();
	auto effectstate = owner->GetComponent<StateEffect>();
	if (!effectstate)return;
	if (!thermal)return;
	switch (thermal->GetHeat())
	{
	case -2:
		effectstate->enabled = true;
		effectstate->SetState("ice");
		break;
	case 1:
		effectstate->enabled = false;
		break;
	default:
		break;
	}
	currentTemp = temp->GetHeat();
}

void water::DrawInspector()
{
}

void water::Serialize(nlohmann::json& j) const
{
}

void water::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> water::Clone() const
{
	return std::make_unique<water>();
}
