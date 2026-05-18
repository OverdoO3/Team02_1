#include "water.h"
#include "Actor.h"
#include "Factory.h"
#include "StateEffect.h"
//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::water, water)

void water::OnAwake(float elapsedTime)
{

}

void water::Update(float elapsedTime)
{
	auto temp = owner->GetComponent<ThermalBody>();
	if (!temp)return;
	auto modelrender = owner->GetComponent<ModelRender>();
	if (!modelrender)return;

	auto col = owner->GetComponent<BoxCollider>();

	std::unique_ptr<Model> model;
	switch (temp->GetHeat())
	{
	case -2:
 		model = std::make_unique<Model>(icepath.c_str());
		col->size.y = 10;
		modelrender->SetModel(std::move(model));
		break;
	default:
		model = std::make_unique<Model>(waterpath.c_str());
		col->size.y = 30;
		modelrender->SetModel(std::move(model));
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
	default:
		effectstate->enabled = false;
		break;
	}
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
