#include "player.h"
#include "Actor.h"
#include "Factory.h"
#include "System/Model.h"
#include <System/GpuResourceUtils.h>

//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::playerModelChanger, playerModelChanger)

void playerModelChanger::OnAwake(float elapsedTime)
{
}

void playerModelChanger::Update(float elapsedTime)
{
	auto thermal = owner->GetComponent<ThermalBody>()->GetHeat();
	if (!thermal)return;
	auto model = owner->GetComponent<ModelRender>();

	if (thermal != currentTemp)
	{
		std::vector<Model::Node> node = model->GetModel()->GetNodes();
		switch (thermal)
		{
		case -2:
			model->SetPlayerTexture(icePath.c_str());
			break;
		case -1:
			model->SetPlayerTexture(waterPath.c_str());
			break;
		case 0:
			model->SetPlayerTexture(normalPath.c_str());
			break;
		case 1:
			model->SetPlayerTexture(hotPath.c_str());
			break;
		default:
			break;
		}
		currentTemp = thermal;
	}
}

void playerModelChanger::DrawInspector()
{
}

void playerModelChanger::Serialize(nlohmann::json& j) const
{
}

void playerModelChanger::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> playerModelChanger::Clone() const
{
	return std::make_unique<playerModelChanger>();
}
