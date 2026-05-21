#include "player.h"
#include "Actor.h"
#include "Factory.h"

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
		std::unique_ptr<Model> newModel = nullptr;
		switch (thermal)
		{
		case -2:
			newModel = std::make_unique<Model>(icePath.c_str());
			model->SetModel(std::move(newModel));
			break;
		case -1:
			newModel = std::make_unique<Model>(waterPath.c_str());
			model->SetModel(std::move(newModel));
			break;
		case 0:
			newModel = std::make_unique<Model>(normalPath.c_str());
			model->SetModel(std::move(newModel));
			break;
		case 1:
			newModel = std::make_unique<Model>(hotPath.c_str());
			model->SetModel(std::move(newModel));
			break;
		default:
			break;
		}
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
