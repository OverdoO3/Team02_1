#include "firewood.h"
#include "Factory.h"
#include "Actor.h"
#include <EffectRender.h>

//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::firewood, firewood)

void firewood::OnAwake(float elapsedTime)
{
}

void firewood::Update(float elapsedTime)
{
	auto t = owner->GetComponent<ThermalBody>();
	if (!t)return;
	auto r = owner->GetComponent<HeatReceiver>();

	if (t->GetHeat() != 1&&onfire)
	{
		onfire = false;
		r->SetHeatNum(0);
		auto eff = owner->GetComponent<EffectRender>();
		eff->Stop();
	}
}

void firewood::DrawInspector()
{
}

void firewood::Serialize(nlohmann::json& j) const
{
}

void firewood::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> firewood::Clone() const
{
	return std::make_unique<firewood>();
}
