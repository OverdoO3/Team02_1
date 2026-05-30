#include "HeatComponent.h"
#include "Actor.h"
#include "Factory.h"
#include "Scene.h"
#include "Collision.h"
#include "StateEffect.h"
#include "ThermalBody.h"
#include <EffectRender.h>
#include "System/Audio.h"

REGISTER_COMPONENT(ComponentID::HeatTransfer, HeatTransfer)

void HeatTransfer::OnAwake(float elapsedTime)
{

}

void HeatTransfer::Update(float elapsedTime)
{
	auto thermal = owner->GetComponent<ThermalBody>();
	auto scene = owner->GetScene();
	if (!thermal) return;

	m_insideActors.clear();


	for (auto& actor : scene->actors)
	{
		auto toThermal = actor->GetComponent<ThermalBody>();
		if (!toThermal) continue;
		if (toThermal == thermal) continue;
		auto toTransform = actor->GetComponent<Transform>();
		auto transform = owner->GetComponent<Transform>();

		if (actor->tag == 1)continue;
		if (Collision::IntersectSphereVsSphere(transform->GetWorldPosition(),
			thermal->GetRadius(),
			toTransform->GetWorldPosition(),
			toThermal->GetRadius()))
		{
			if(thermal->GetHeat() != 0)
			toThermal->SetHeat(thermal->GetHeat());
			m_insideActors.push_back(actor.get());
		} 
	}
	m_canAbsorb = false;
	for (auto& actor : scene->actors)
	{
		auto Receiver = actor->GetComponent<HeatReceiver>();
		if (!Receiver) continue;
		auto toTransform = actor->GetComponent<Transform>();
		auto transform = owner->GetComponent<Transform>();

		if (Collision::IntersectSphereVsSphere(transform->GetWorldPosition(),
			thermal->GetRadius(),
			toTransform->GetWorldPosition(),
			Receiver->GetRadius()))
		{
			m_canAbsorb = true;
			break; // 一つでも範囲内ならUIフラグONでOK
		}
	}

	
	if (InputC::KeyPressed(VK_LBUTTON))
	{
		bool success = false;
		for (auto& actor : scene->actors)
		{
			auto Receiver = actor->GetComponent<HeatReceiver>();
			if (!Receiver) continue;
			auto toTransform = actor->GetComponent<Transform>();
			auto transform = owner->GetComponent<Transform>();

			if (Collision::IntersectSphereVsSphere(transform->GetWorldPosition(),
				thermal->GetRadius(),
				toTransform->GetWorldPosition(),
				Receiver->GetRadius()))
			{
				thermal->SetHeat(Receiver->GetHeatNum());
				auto th = actor->GetComponent<ThermalBody>();
				if (th->GetHeat() == 1)
				{
					th->SetHeat(0);
					auto a = actor->GetComponent<EffectRender>();
					if (a)
					{
						a->Stop();
					}
				}
				success = true;
			}
		}
		if (!success && thermal->GetHeat() != 0)
		{
			auto m = owner->GetComponent<ModelRender>();
			if (m)
			{
				m->PlayAnimation("in_out", false);
			}

			auto child = owner->GetChildren();
			switch (thermal->GetHeat())
			{
			case -2:
			{
				if (!child.empty())
				{
					child.front()->GetComponent<StateEffect>()->SetState("icebreath");
				}

				std::string myPath = ToDataPath("Data/Sound/SE_game_breath_ice.wav");
				Audio::Instance().PlaySE(myPath.c_str());
			}
				break;
			case -1:
			{
				if (!child.empty())
				{
					child.front()->GetComponent<StateEffect>()->SetState("waterbreath");
				}
				std::string myPath = ToDataPath("Data/Sound/SE_game_breath_water.wav");
				Audio::Instance().PlaySE(myPath.c_str());
				break;
			}
			case 1:
			{
				if (!child.empty())
				{
					child.front()->GetComponent<StateEffect>()->SetState("hotbreath");
				}
				std::string myPath = ToDataPath("Data/Sound/SE_game_breath_fire.wav");
				Audio::Instance().PlaySE(myPath.c_str());
			}
			break;

			default:
				break;
			}
			if (!child.empty())
			{
				owner->GetChildren().front()->GetComponent<ThermalBody>()->SetHeat(thermal->GetHeat());
			}
			
			thermal->SetHeat(0);
		}
	}


	auto effectstate = owner->GetComponent<StateEffect>();
	if (!effectstate) return;

	if (m_prevHeat != thermal->GetHeat())
	{
		switch (thermal->GetHeat())
		{
		case 1:
		{
			effectstate->SetState("hot");
			std::string myPath = ToDataPath("Data/Sound/SE_game_reaction_fire.wav");
			Audio::Instance().PlaySE(myPath.c_str());
		}
		break;
		case -1:
		{
			effectstate->SetState("water");
			std::string myPath = ToDataPath("Data/Sound/SE_game_reaction_water.wav");
			Audio::Instance().PlaySE(myPath.c_str());
		}
		break;
		case -2:
		{
			effectstate->SetState("cold");
			std::string myPath = ToDataPath("Data/Sound/SE_game_reaction_ice.wav");
			Audio::Instance().PlaySE(myPath.c_str());
		}
		break;
		case 0:
		{
			effectstate->SetState("normal");
		}
		break;
		}

		// 最後に現在の状態を保存して、次フレームと比較できるようにする
		m_prevHeat = thermal->GetHeat();
	}
}

void HeatTransfer::DrawInspector()
{

}

void HeatTransfer::Serialize(nlohmann::json& j) const
{

}

void HeatTransfer::Deserialize(nlohmann::json& j)
{

}

std::unique_ptr<Component> HeatTransfer::Clone() const
{
	return std::make_unique<HeatTransfer>();
}

bool HeatTransfer::IsStatusActive() const
{
	auto thermal = owner->GetComponent<ThermalBody>();
	if (!thermal) return false;

	return (thermal->GetHeat() != 0);
}