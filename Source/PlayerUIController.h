#pragma once
#include "Component.h"
#include "ThermalBody.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class PlayerUIController : public Component
{
public:
	void Update(float elapsedTime)override;
	void DrawInspector()override;

	std::unique_ptr<Component> Clone() const override;
	void Serialize(nlohmann::json& j) const override;
	void Deserialize(nlohmann::json& j) override;

	//ComponentID GetID() const override { return ComponentID::PlayerUIController; }

	COMPONENT_ID(PlayerUIController)

private:
	int m_lastHeat = 999;
	ThermalBody* m_playerThermal = nullptr;
};