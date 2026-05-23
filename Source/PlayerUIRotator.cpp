#include "PlayerUIRotator.h"
#include "Actor.h"
#include "Scene.h"
#include "SpriteRender.h"
#include "ThermalBody.h"
#include <Lerp.h>

REGISTER_COMPONENT(ComponentID::PlayerUIRotator, PlayerUIRotator)

void PlayerUIRotator::Update(float elapsedTime)
{
	//まだプレイヤーのThermalBodyを保持していなければ探しに行く
	if (!m_playerThermal)
	{
		auto scene = owner->GetScene();
		if (scene)
		{
			for (auto& actor : scene->actors)
			{
				if (actor->tag == 1)
				{
					// プレイヤーが見つかったら ThermalBody コンポーネントを取得して保存
					m_playerThermal = actor->GetComponent<ThermalBody>();
					break;
				}
			}
		}
	}
	if (m_playerThermal)
	{
		int heat = m_playerThermal->GetHeat();
		switch (heat)
		{
		//case -2: //氷
		//	m_targetAngle = 180.0f;
		//	break;
		//case -1: // 水
		//	m_targetAngle = 270.0f;
		//	break;
		//case 0:  // 無
		//	m_targetAngle = 0.0f;
		//	break;
		//case 1:  // 火
		//	m_targetAngle = 90.0f;
		//	break;
		//default:
		//	break;
		case -2: //氷
			m_targetAngle = 135.0f;
			break;
		case -1: // 水
			m_targetAngle = 225.0f;
			break;
		case 0:  // 無
			m_targetAngle = -45.0f;
			break;
		case 1:  // 火
			m_targetAngle =	45.0f;
			break;
		default:
			break;

		}
	}

	auto sprite = owner->GetComponent<SpriteRender>();
	if (sprite)
	{
		// std::lerp を使って現在の角度を目標角度へじわじわ近づける
		m_currentAngle = Lerp(m_currentAngle, m_targetAngle, 0.1f);

		sprite->SetAngle(m_currentAngle);
	}
}

void PlayerUIRotator::DrawInspector()
{
	ImGui::DragFloat("Current Angle", &m_currentAngle);
	ImGui::DragFloat("Target Angle", &m_targetAngle);

	if (m_playerThermal) {
		ImGui::Text("Player Heat: %d", m_playerThermal->GetHeat());
	}
	else {
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Player Not Found!");
	}
}

std::unique_ptr<Component> PlayerUIRotator::Clone() const
{
	auto c = std::make_unique<PlayerUIRotator>();
	c->m_currentAngle = m_currentAngle;
	c->m_targetAngle = m_targetAngle;
	return c;
}

void PlayerUIRotator::Serialize(json& j) const
{
	j["CurrentAngle"] = m_currentAngle;
	j["TargetAngle"] = m_targetAngle;
}

void PlayerUIRotator::Deserialize(json& j)
{
	m_currentAngle = j.value("CurrentAngle", 0.0f);
	m_targetAngle = j.value("TargetAngle", 0.0f);
}