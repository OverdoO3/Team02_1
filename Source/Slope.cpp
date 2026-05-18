#include "Slope.h"
#include "Actor.h"
#include "Factory.h"

//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::Slope, Slope)

void Slope::OnAwake(float elapsedTime)
{
	angle = DirectX::XM_PI / 4.0f;
}

void Slope::Update(float elapsedTime)
{
	auto t = owner->GetComponent<Transform>();
	dirYaw = t->GetEulerRotation().y;
}

void Slope::DrawInspector()
{
	ImGui::InputFloat("dirYaw", &dirYaw);
}

void Slope::Serialize(nlohmann::json& j) const
{
}

void Slope::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> Slope::Clone() const
{
	auto slope = std::make_unique<Slope>();
	return slope;
}
