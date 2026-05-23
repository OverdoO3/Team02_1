#include "ThermalBody.h"
#include "Factory.h"
#include "Actor.h"

//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::ThermalBody, ThermalBody)

void ThermalBody::OnAwake(float elapsedTime)
{
}

void ThermalBody::Update(float elapsedTime)
{
}

void ThermalBody::DrawInspector()
{
	ImGui::InputInt("temprature",&temperature);
	ImGui::InputFloat("radius", &radius);
}

void ThermalBody::Serialize(nlohmann::json& j) const
{
	j["radius"] = radius;
	j["thermal"] = temperature;
}

void ThermalBody::Deserialize(nlohmann::json& j)
{
	radius =  j["radius"];
	if(j["thermal"] != nullptr)
	temperature = j["thermal"];
}

void ThermalBody::RenderDebug(RenderContext& rc, ShapeRenderer* renderer)
{
	auto transform = owner->GetComponent<Transform>();

	switch (temperature)
	{
	case 0:
		color = { 1,1,1,1 };
		break;
	case 1:
		color = { 1,0,0,1 };
		break;
	case -1:
		color = { 0,0,1,1 };
		break;
	case -2:
		color = { 0,1,1,1 };
		break;
	default:
		break;
	}
	renderer->RenderSphere(rc, transform->GetWorldPosition(), radius, color);
}

std::unique_ptr<Component> ThermalBody::Clone() const
{
	auto c = std::make_unique<ThermalBody>();
	c->SetHeat(temperature);
	c->radius = radius;
	return c;
}
