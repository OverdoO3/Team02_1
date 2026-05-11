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

	std::unique_ptr<Model> model;
	switch (temp->GetHeat())
	{
	case -1:
		model = std::make_unique<Model>(icepath.c_str());
		modelrender->SetModel(std::move(model));
		break;
	case 0:
		model = std::make_unique<Model>(waterpath.c_str());
		modelrender->SetModel(std::move(model));
		break;
	case 1:
		model = std::make_unique<Model>(waterpath.c_str());
		modelrender->SetModel(std::move(model));
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
	case 1:
	case 0:
		effectstate->enabled = false;
		break;
	case -1:
		effectstate->enabled = true;
		effectstate->SetState("ice");
		break;
	default:
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
