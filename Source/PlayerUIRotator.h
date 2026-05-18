#pragma once
#include "Component.h"
#include "ThermalBody.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class PlayerUIRotator : public Component
{
public:
	void Update(float elapsedTime)override;
	void DrawInspector()override;

	std::unique_ptr<Component> Clone() const override;
	void Serialize(nlohmann::json& j) const override;
	void Deserialize(nlohmann::json& j) override;

	//ComponentID GetID() const override { return ComponentID::PlayerUIRotator; }

	COMPONENT_ID(PlayerUIRotator)

private:
	float m_currentAngle = 0.0f;
	float m_targetAngle = 0.0f;
	ThermalBody* m_playerThermal = nullptr;
};